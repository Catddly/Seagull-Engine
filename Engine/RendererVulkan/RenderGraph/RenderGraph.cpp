#include "StdAfx.h"
#include "RenderGraph.h"

#include "System/Logger.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"

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
		mRenderGraph.Build();
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/// RenderGraph
	/////////////////////////////////////////////////////////////////////////////////////

	RenderGraph::~RenderGraph()
	{
		Clear();
	}

	void RenderGraph::Draw(UInt32 frameIndex) const
	{
		auto* pFrameBuffer = mpFrameBuffers[frameIndex];
		auto& commandBuf = mpRenderContext->commandBuffers[frameIndex];
		auto* pColorRt = mpRenderContext->colorRts[frameIndex];

		commandBuf.BeginRecord();
		ClearValue cv;
		cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		cv.depthStencil = { 1.0f, 0 };
		
		commandBuf.SetViewport((float)pColorRt->width, (float)pColorRt->height, 0.0f, 1.0f);
		commandBuf.SetScissor({ 0, 0, (int)pColorRt->width, (int)pColorRt->height });

		auto* pCurrNode = mpRootNode;
		while (pCurrNode)
		{
			commandBuf.BeginRenderPass(pFrameBuffer, cv);
			pCurrNode->Execute(commandBuf);
			commandBuf.EndRenderPass();
			pCurrNode = pCurrNode->mpNext;
		}
		commandBuf.EndRecord();
	}

	void RenderGraph::WindowResize()
	{
		for (UInt32 i = 0; i < mpRenderContext->swapchain.imageCount; ++i) // recreate the frame buffers
		{
			Memory::Delete(mpFrameBuffers[i]);
			VulkanFrameBuffer** ppFrameBuffer = &mpFrameBuffers[i];
			*ppFrameBuffer = VulkanFrameBuffer::Builder(mpRenderContext->device)
				.AddRenderTarget(mpRenderContext->colorRts[i])
				.AddRenderTarget(mpRenderContext->depthRt)
				.BindRenderPass(mpCurrRenderPass)
				.Build();
		}
	}

	void RenderGraph::Build()
	{
		auto* pCurrNode = mpRootNode;
		while (pCurrNode)
		{
			// TODO: renderpass belong to this node (cache it).
			mpCurrRenderPass = pCurrNode->Prepare();
			pCurrNode = pCurrNode->mpNext;
		}

		mpFrameBuffers.resize(mpRenderContext->swapchain.imageCount);
		for (UInt32 i = 0; i < mpRenderContext->swapchain.imageCount; ++i)
		{
			VulkanFrameBuffer** ppFrameBuffer = &mpFrameBuffers[i];
			*ppFrameBuffer = VulkanFrameBuffer::Builder(mpRenderContext->device)
				.AddRenderTarget(mpRenderContext->colorRts[i])
				.AddRenderTarget(mpRenderContext->depthRt)
				.BindRenderPass(mpCurrRenderPass)
				.Build();
		}
	}

	void RenderGraph::Clear()
	{
		auto* pCurrNode = mpRootNode;
		while (pCurrNode)
		{
			pCurrNode->Clear();
			auto* pNode = pCurrNode;
			pCurrNode = pCurrNode->mpNext;
			Memory::Delete(pNode);
		}

		for (UInt32 i = 0; i < mpRenderContext->swapchain.imageCount; ++i)
			Memory::Delete(mpFrameBuffers[i]);
	}

}