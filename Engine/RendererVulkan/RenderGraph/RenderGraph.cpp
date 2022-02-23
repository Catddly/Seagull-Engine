#include "StdAfx.h"
#include "RenderGraph.h"

#include "System/Logger.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"

#include "Stl/Hash.h"

namespace SG
{

	/////////////////////////////////////////////////////////////////////////////////////
	/// RenderGraphBuilder
	/////////////////////////////////////////////////////////////////////////////////////

	RenderGraphBuilder& RenderGraphBuilder::NewRenderPass(RenderGraphNode* pNode)
	{
		if (!mRenderGraph.mpRootNode)
			mRenderGraph.mpRootNode = pNode;
		else
		{
			auto* pCurrNode = mRenderGraph.mpRootNode;
			while (true)
			{
				if (pCurrNode->mpPrev)
					pCurrNode = pCurrNode->mpPrev;
				else
					pCurrNode->mpPrev = pNode;
			}	
		}
		return *this;
	}

	void RenderGraphBuilder::Complete()
	{
		//mRenderGraph.Compile();
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/// RenderGraph
	/////////////////////////////////////////////////////////////////////////////////////

	RenderGraph::RenderGraph(const char* name, VulkanContext* pContext)
		: mName(name), mpRootNode(nullptr), mpRenderContext(pContext)
	{
	}

	RenderGraph::~RenderGraph()
	{
		auto* pCurrNode = mpRootNode;
		while (pCurrNode)
		{
			auto* pNode = pCurrNode;
			pCurrNode = pCurrNode->mpNext;
			Memory::Delete(pNode);
		}

		// Delete all the render pass and frame buffer
		for (auto& beg = mRenderPassesMap.begin(); beg != mRenderPassesMap.end(); ++beg)
			Memory::Delete(beg->second);
		for (auto& beg = mFrameBuffersMap.begin(); beg != mFrameBuffersMap.end(); ++beg)
			Memory::Delete(beg->second);
	}

	void RenderGraph::Update()
	{
		auto* pCurrNode = mpRootNode;
		while (pCurrNode) // iterate all nodes
		{
			pCurrNode->Update(mFrameIndex);
			pCurrNode = pCurrNode->mpNext;
		}

		Compile();
	}

	void RenderGraph::Draw(UInt32 frameIndex) const
	{
		SG_ASSERT(!mFrameBuffersMap.empty() && "RenderGraphBuilder should call Complete() or RenderGraph should call Compile() after the node insertion!");

		auto& commandBuf = mpRenderContext->commandBuffers[frameIndex];
		auto* pColorRt = mpRenderContext->colorRts[frameIndex];

		Size framebufferHash = 0;
		UInt32 colorRtAddress[3] = {
			pColorRt->GetID(),
			pColorRt->GetNumMipmap(),
			pColorRt->GetNumArray(),
		};
		framebufferHash = HashMemory32Array(colorRtAddress, 3, framebufferHash);
		UInt32 depthRtAddress[3] = {
			mpRenderContext->depthRt->GetID(),
			mpRenderContext->depthRt->GetNumMipmap(),
			mpRenderContext->depthRt->GetNumArray(),
		};
		framebufferHash = HashMemory32Array(depthRtAddress, 3, framebufferHash);
		auto* pFrameBuffer = mFrameBuffersMap.find(framebufferHash)->second;

		auto* pCurrNode = mpRootNode;
		commandBuf.BeginRecord();
		while (pCurrNode)
		{
			ClearValue cv;
			cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			cv.depthStencil = { 1.0f, 0 };

			commandBuf.SetViewport((float)pColorRt->GetWidth(), (float)pColorRt->GetHeight(), 0.0f, 1.0f);
			commandBuf.SetScissor({ 0, 0, (int)pColorRt->GetWidth(), (int)pColorRt->GetHeight() });

			commandBuf.BeginRenderPass(pFrameBuffer, cv);
			pCurrNode->Execute(commandBuf);
			commandBuf.EndRenderPass();
			pCurrNode = pCurrNode->mpNext;
		}
		commandBuf.EndRecord();

		mFrameIndex = (frameIndex + 1) % mpRenderContext->swapchain.imageCount;
	}

	void RenderGraph::WindowResize()
	{
		mpRenderContext->device.WaitIdle();

		for (auto& beg = mFrameBuffersMap.begin(); beg != mFrameBuffersMap.end(); ++beg)
			Memory::Delete(beg->second);
		mFrameBuffersMap.clear();

		// after the resizing, all the render targets had been recreated,
		// update the node to update the resources which is using in the node.
		auto* pCurrNode = mpRootNode;
		while (pCurrNode) // iterate all nodes
		{
			pCurrNode->Reset();
			pCurrNode = pCurrNode->mpNext;
		}
		mFrameIndex = 0;
	}

	void RenderGraph::Compile()
	{
		auto* pCurrNode = mpRootNode;
		while (pCurrNode) // iterate all nodes
		{
			if (pCurrNode->mInResources.empty())
			{
				SG_LOG_ERROR("No resource bound to this RenderGraphNode!");
				SG_ASSERT(false);
			}

			// calculate the hash data
			Size renderpassHash = 0;
			Size framebufferHash = 0;
			for (auto& resource : pCurrNode->mInResources)
			{
				renderpassHash = resource.GetDataHash(renderpassHash);

				UInt32 address[3] = {
					resource.mpRenderTarget->GetID(),
					resource.mpRenderTarget->GetNumMipmap(),
					resource.mpRenderTarget->GetNumArray(),
				};
				framebufferHash = HashMemory32Array(address, 3, framebufferHash);
			}

			// retrieve the render pass of this node
			auto& pRenderPassNode = mRenderPassesMap.find(renderpassHash);
			if (pRenderPassNode == mRenderPassesMap.end()) // Didn't find this render pass, then create a new one
			{
				VulkanRenderPass::Builder rpBuilder(mpRenderContext->device);
				for (auto& resource : pCurrNode->mInResources)
				{
					rpBuilder.BindRenderTarget(resource.mpRenderTarget, resource.mLoadStoreClearOp,
						EResourceBarrier::efUndefined, 
						resource.mpRenderTarget->IsDepth() ? EResourceBarrier::efDepth_Stencil : EResourceBarrier::efPresent);
				}
				mpCurrRenderPass = rpBuilder.CombineAsSubpass().Build();

				mRenderPassesMap[renderpassHash] = mpCurrRenderPass; // append the new render pass to the map
			}
			else // this render pass already exist, just use it
			{
				mpCurrRenderPass = pRenderPassNode->second;
			}

			// create the framebuffer of this node if it doesn't exist
			auto& pFrameBufferNode = mFrameBuffersMap.find(framebufferHash);
			if (pFrameBufferNode == mFrameBuffersMap.end()) // Didn't find this framebuffer, then create a new one
			{
				VulkanFrameBuffer::Builder fbBuilder(mpRenderContext->device);
				for (auto& resource : pCurrNode->mInResources)
					fbBuilder.AddRenderTarget(resource.mpRenderTarget);
				auto* pFrameBuffer = fbBuilder.BindRenderPass(mpCurrRenderPass).Build();

				mFrameBuffersMap[framebufferHash] = pFrameBuffer; // append the new framebuffer to the map
			}

			pCurrNode->Prepare(mpCurrRenderPass);

			pCurrNode = pCurrNode->mpNext;
		}
	}

}