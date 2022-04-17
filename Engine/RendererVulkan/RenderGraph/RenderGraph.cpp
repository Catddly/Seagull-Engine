#include "StdAfx.h"
#include "RenderGraph.h"

#include "System/Logger.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "Stl/Hash.h"

namespace SG
{

	/////////////////////////////////////////////////////////////////////////////////////
	/// RenderGraphBuilder
	/////////////////////////////////////////////////////////////////////////////////////

	RenderGraphBuilder::RenderGraphBuilder(const char* name, VulkanContext* pContext)
		:mpContext(pContext)
	{
		mpRenderGraph = MakeUnique<RenderGraph>(name, pContext);
	}

	RenderGraphBuilder::~RenderGraphBuilder()
	{
		if (!mbInitSuccess)
		{
			for (auto* pNode : mpRenderGraph->mpNodes)
				Memory::Delete(pNode);
			mpRenderGraph->mpNodes.clear();
			mpRenderGraph.reset(nullptr);
		}
	}

	UniquePtr<RenderGraph> RenderGraphBuilder::Build()
	{
		mpRenderGraph->Compile();
		mbInitSuccess = true;
		return eastl::move(mpRenderGraph);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/// RenderGraph
	/////////////////////////////////////////////////////////////////////////////////////

	RenderGraph::RenderGraph(const char* name, VulkanContext* pContext)
		: mName(name), mpContext(pContext)
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
			pCurrNode->Update();
	}

	void RenderGraph::Draw(UInt32 frameIndex) const
	{
		SG_ASSERT(!mFrameBuffersMap.empty() && "RenderGraphBuilder should call Build() or RenderGraph should call Compile() after the node insertion!");

		auto& commandBuf = mpContext->commandBuffers[frameIndex];
		RenderGraphNode::RGDrawInfo drawContext = { &commandBuf, frameIndex };

		commandBuf.BeginRecord();
		for (auto* pCurrNode : mpNodes)
		{
			// calculate the hash data
			Size framebufferHash = 0;
			for (auto& resource : pCurrNode->mInResources)
			{
				if (resource.has_value())
				{
					bool bHaveMultiResource = false;
					if (resource->GetNumRenderTarget() != 1)
						bHaveMultiResource = true;

					UInt32 address[3] = {
						resource->GetRenderTarget(bHaveMultiResource ? frameIndex : 0)->GetID(),
						resource->GetRenderTarget(bHaveMultiResource ? frameIndex : 0)->GetNumMipmap(),
						resource->GetRenderTarget(bHaveMultiResource ? frameIndex : 0)->GetNumArray(),
					};
					framebufferHash = HashMemory32Array(address, 3, framebufferHash);
				}
			}
			auto* pFrameBuffer = mFrameBuffersMap.find(framebufferHash)->second;

			commandBuf.BeginRenderPass(pFrameBuffer);
			pCurrNode->Draw(drawContext);
			commandBuf.EndRenderPass();
		}
		commandBuf.EndRecord();
	}

	void RenderGraph::WindowResize()
	{
		mpContext->graphicQueue.WaitIdle();

		for (auto& beg = mFrameBuffersMap.begin(); beg != mFrameBuffersMap.end(); ++beg)
			Memory::Delete(beg->second);
		mFrameBuffersMap.clear();

		// after the resizing, all the render targets had been recreated,
		// reset the node to update the resources which is using.
		for (auto* pCurrNode : mpNodes)
		{
			pCurrNode->Reset();

			if (!pCurrNode->HaveValidResource())
			{
				SG_LOG_ERROR("No resource bound to this RenderGraphNode!");
				SG_ASSERT(false);
			}

			CompileFrameBuffers(pCurrNode);
		}
	}

	void RenderGraph::Compile()
	{
		for (auto* pCurrNode : mpNodes) // iterate all nodes
		{
			if (!pCurrNode->HaveValidResource())
			{
				SG_LOG_ERROR("No resource bound to this RenderGraphNode!");
				SG_ASSERT(false);
			}

			auto* pRenderPass = CompileRenderPasses(pCurrNode);
			CompileFrameBuffers(pCurrNode);

			pCurrNode->Prepare(pRenderPass);
		}

		// wait for all the resource in the transfer queue.
		VK_RESOURCE()->WaitBuffersUpdate();
	}

	VulkanRenderPass* RenderGraph::CompileRenderPasses(const RenderGraphNode* pCurrNode)
	{
		Size renderpassHash = 0;
		for (auto& resource : pCurrNode->mInResources)
		{
			if (resource.has_value())
			{
				// calculate the hash value
				renderpassHash = resource->GetRenderPassHash(renderpassHash);
			}
		}

		// retrieve the render pass of this node
		auto& pRenderPassNode = mRenderPassesMap.find(renderpassHash);
		if (pRenderPassNode == mRenderPassesMap.end()) // Didn't find this render pass, then create a new one
		{
			VulkanRenderPass::Builder rpBuilder(mpContext->device);
			for (auto& resource : pCurrNode->mInResources)
			{
				if (resource.has_value())
				{
					auto statusTransition = mResourceStatusKeeper.GetResourceNextStatus(resource->GetRenderTarget());
					rpBuilder.BindRenderTarget(resource->GetRenderTarget(), resource->GetLoadStoreClearOp(),
						statusTransition.srcStatus, statusTransition.dstStatus);
				}
			}
			auto* pRenderPass = rpBuilder.CombineAsSubpass().Build();
			mRenderPassesMap[renderpassHash] = pRenderPass; // append the new render pass to the map
			return pRenderPass;
		}
		return pRenderPassNode->second;
	}

	void RenderGraph::CompileFrameBuffers(const RenderGraphNode* pCurrNode)
	{
		// Each Render Graph Resource Should Have its own FrameBuffer!
		UInt32 maxNum = 1;
		for (auto& resource : pCurrNode->mInResources)
		{
			if (resource.has_value())
			{
				maxNum = eastl::max(maxNum, resource->GetNumRenderTarget());
			}
		}

		// calculate the hash data
		UInt32 currIndex = 0;
		for (UInt32 i = 0; i < maxNum; ++i)
		{
			Size framebufferHash = 0;
			Size renderpassHash = 0;
			vector<eastl::pair<VulkanRenderTarget*, ClearValue>> frameRtCollection;

			for (auto& resource : pCurrNode->mInResources)
			{
				if (resource.has_value())
				{
					bool bHaveMultiResource = false;
					if (resource->GetNumRenderTarget() != 1)
						bHaveMultiResource = true;

					frameRtCollection.emplace_back(resource->GetRenderTarget(bHaveMultiResource ? currIndex : 0), resource->GetClearValue());

					renderpassHash = resource->GetRenderPassHash(renderpassHash);
					framebufferHash = resource->GetFrameBufferHash(framebufferHash, bHaveMultiResource ? currIndex : 0);
				}
			}

			// create the framebuffer of this node if it doesn't exist
			auto& pFrameBufferNode = mFrameBuffersMap.find(framebufferHash);
			if (pFrameBufferNode == mFrameBuffersMap.end()) // Didn't find this framebuffer, then create a new one
			{
				VulkanFrameBuffer::Builder fbBuilder(mpContext->device);
				for (auto& resource : frameRtCollection)
					fbBuilder.AddRenderTarget(resource.first, resource.second);

				auto* pRenderPass = mRenderPassesMap.find(renderpassHash)->second;
				auto* pFrameBuffer = fbBuilder.BindRenderPass(pRenderPass).Build();

				mFrameBuffersMap[framebufferHash] = pFrameBuffer; // append the new framebuffer to the map
			}
			++currIndex;
		}
	}

	void RenderGraph::ResetFrameBuffer(RenderGraphNode* pNode, Size frameBufferHash) noexcept
	{
		auto node = mFrameBuffersMap.find(frameBufferHash);
		if (node != mFrameBuffersMap.end())
		{
			Memory::Delete(node->second);
			mFrameBuffersMap.erase(node);
		}
		CompileFrameBuffers(pNode);
	}

	void RenderGraph::AddResourceDenpendency(VulkanRenderTarget* pRenderTarget, EResourceBarrier srcStatus, EResourceBarrier dstStatus)
	{
		mResourceStatusKeeper.AddResourceDenpendency(pRenderTarget, srcStatus, dstStatus);
	}

	void RenderGraph::RemoveResourceDenpendency(VulkanRenderTarget* pRenderTarget)
	{
		mResourceStatusKeeper.RemoveResourceDenpendency(pRenderTarget);
	}

}