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
		mRenderGraph.mpNodes.push_back(pNode);
		return *this;
	}

	void RenderGraphBuilder::Complete()
	{
		mRenderGraph.Compile();
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/// RenderGraph
	/////////////////////////////////////////////////////////////////////////////////////

	RenderGraph::RenderGraph(const char* name, VulkanContext* pContext)
		: mName(name), mpRenderContext(pContext)
	{
	}

	RenderGraph::~RenderGraph()
	{
		for (auto* pNode : mpNodes)
			Memory::Delete(pNode);
		mpNodes.clear();

		// Delete all the render pass and frame buffer
		for (auto& beg = mRenderPassesMap.begin(); beg != mRenderPassesMap.end(); ++beg)
			Memory::Delete(beg->second);
		for (auto& beg = mFrameBuffersMap.begin(); beg != mFrameBuffersMap.end(); ++beg)
			Memory::Delete(beg->second);
	}

	void RenderGraph::Update()
	{
		for (auto* pCurrNode : mpNodes)
		{
			pCurrNode->Update(mFrameIndex);

			// after the update, if some node is changed, compile it.
			CompileRenderPasses(pCurrNode);
			CompileFrameBuffers(pCurrNode);
		}
	}

	void RenderGraph::Draw(UInt32 frameIndex) const
	{
		SG_ASSERT(!mFrameBuffersMap.empty() && "RenderGraphBuilder should call Complete() or RenderGraph should call Compile() after the node insertion!");

		auto& commandBuf = mpRenderContext->commandBuffers[frameIndex];
		auto* pColorRt = mpRenderContext->colorRts[frameIndex];

		RenderGraphNode::RGDrawContext drawContext = { &commandBuf, frameIndex };

		commandBuf.BeginRecord();
		for (auto* pCurrNode : mpNodes)
		{
			// calculate the hash data
			Size framebufferHash = 0;
			for (auto& resource : pCurrNode->mInResources)
			{
				if (resource.has_value())
				{
					UInt32 address[3] = {
						resource->mpRenderTarget->GetID(),
						resource->mpRenderTarget->GetNumMipmap(),
						resource->mpRenderTarget->GetNumArray(),
					};
					framebufferHash = HashMemory32Array(address, 3, framebufferHash);
				}
			}
			auto* pFrameBuffer = mFrameBuffersMap.find(framebufferHash)->second;

			ClearValue cv;
			cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			cv.depthStencil = { 1.0f, 0 };

			commandBuf.BeginRenderPass(pFrameBuffer, cv);
			pCurrNode->Execute(drawContext);
			commandBuf.EndRenderPass();
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
		for (auto* pCurrNode : mpNodes)
			pCurrNode->Reset();
		mFrameIndex = 0;
	}

	void RenderGraph::Compile()
	{
		for (auto* pCurrNode : mpNodes) // iterate all nodes
		{
			pCurrNode->Update(mFrameIndex);

			if (!pCurrNode->HaveValidResource())
			{
				SG_LOG_ERROR("No resource bound to this RenderGraphNode!");
				SG_ASSERT(false);
			}
			
			CompileRenderPasses(pCurrNode);
			CompileFrameBuffers(pCurrNode);

			pCurrNode->Prepare(mpCurrRenderPass);
		}
	}

	void RenderGraph::CompileRenderPasses(const RenderGraphNode* pCurrNode)
	{
		// TODO: to remove it.
		static UInt64 cnt = 1;

		// calculate the hash data
		Size renderpassHash = 0;
		for (auto& resource : pCurrNode->mInResources)
		{
			if (resource.has_value())
				renderpassHash = resource->GetDataHash(renderpassHash);
		}

		// retrieve the render pass of this node
		auto& pRenderPassNode = mRenderPassesMap.find(renderpassHash);
		if (pRenderPassNode == mRenderPassesMap.end()) // Didn't find this render pass, then create a new one
		{
			VulkanRenderPass::Builder rpBuilder(mpRenderContext->device);
			for (auto& resource : pCurrNode->mInResources)
			{
				if (resource.has_value())
				{
					EResourceBarrier barrier = (cnt % 2 == 1) ? EResourceBarrier::efUndefined : EResourceBarrier::efPresent;
					rpBuilder.BindRenderTarget(resource->mpRenderTarget, resource->mLoadStoreClearOp,
						barrier,
						resource->mpRenderTarget->IsDepth() ? EResourceBarrier::efDepth_Stencil : EResourceBarrier::efPresent);
				}
			}
			++cnt;
			mpCurrRenderPass = rpBuilder.CombineAsSubpass().Build();

			mRenderPassesMap[renderpassHash] = mpCurrRenderPass; // append the new render pass to the map
		}
		else // this render pass already exist, just use it
		{
			mpCurrRenderPass = pRenderPassNode->second;
		}
	}

	void RenderGraph::CompileFrameBuffers(const RenderGraphNode* pCurrNode)
	{
		// calculate the hash data
		Size framebufferHash = 0;
		for (auto& resource : pCurrNode->mInResources)
		{
			if (resource.has_value())
			{
				UInt32 address[3] = {
					resource->mpRenderTarget->GetID(),
					resource->mpRenderTarget->GetNumMipmap(),
					resource->mpRenderTarget->GetNumArray(),
				};
				framebufferHash = HashMemory32Array(address, 3, framebufferHash);
			}
		}

		// create the framebuffer of this node if it doesn't exist
		auto& pFrameBufferNode = mFrameBuffersMap.find(framebufferHash);
		if (pFrameBufferNode == mFrameBuffersMap.end()) // Didn't find this framebuffer, then create a new one
		{
			VulkanFrameBuffer::Builder fbBuilder(mpRenderContext->device);
			for (auto& resource : pCurrNode->mInResources)
			{
				if (resource.has_value())
				{
					fbBuilder.AddRenderTarget(resource->mpRenderTarget);
				}
			}
			auto* pFrameBuffer = fbBuilder.BindRenderPass(mpCurrRenderPass).Build();

			mFrameBuffersMap[framebufferHash] = pFrameBuffer; // append the new framebuffer to the map
		}
	}

}