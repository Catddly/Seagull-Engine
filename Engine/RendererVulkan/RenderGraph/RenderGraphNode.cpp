#include "StdAfx.h"
#include "RenderGraphNode.h"

#include "System/Logger.h"
#include "RendererVulkan/RenderGraph/RenderGraph.h"

#include "RendererVulkan/Backend/VulkanTexture.h"

#include <eastl/algorithm.h>

namespace SG
{

	RenderGraphNode::RenderGraphNode(RenderGraph* pRenderGraph)
		:mResourceValidFlag(0), mpRenderGraph(pRenderGraph)
	{
	}

	UInt32 RenderGraphNode::GetNumResource() const
	{
		UInt32 count = 0;
		for (UInt32 i = 0; i < SG_MAX_RENDER_GRAPH_NODE_RESOURCE; ++i)
		{
			if ((mResourceValidFlag & UInt32(1 << i)) != 0)
				++count;
		}
		return count;
	}

	const RenderGraphNode::InResourceType& RenderGraphNode::GetResource(UInt32 slot)
	{
		SG_ASSERT(slot <= SG_MAX_RENDER_GRAPH_NODE_RESOURCE && "Exceed the maximun limit of render graph node resources.");
		SG_ASSERT((mResourceValidFlag & UInt32(1 << slot)) != 0 && "No resource had been attach to this slot");
		return mInResources[slot];
	}

	void RenderGraphNode::AttachResource(UInt32 slot, const RenderGraphInReousrce& resource)
	{
		SG_ASSERT(slot <= SG_MAX_RENDER_GRAPH_NODE_RESOURCE && "Exceed the maximun limit of render graph node resources.");
		if ((mResourceValidFlag & UInt32(1 << slot)) != 0) // already add a resource to this slot
			mpRenderGraph->RemoveResourceDenpendency(mInResources[slot]->GetRenderTarget());

		mInResources[slot] = resource;
		mResourceValidFlag |= UInt32(1 << slot);
		mpRenderGraph->AddResourceDenpendency(resource.GetRenderTarget(), resource.GetSrcStatus(), resource.GetDstStatus());

		mClearValues[slot] = mInResources[slot]->GetClearValue();
	}

	void RenderGraphNode::DetachResource(UInt32 slot)
	{
		SG_ASSERT(slot <= SG_MAX_RENDER_GRAPH_NODE_RESOURCE && "Exceed the maximun limit of render graph node resources.");
		SG_ASSERT((mResourceValidFlag & UInt32(1 << slot)) != 0 && "No resource had been attach to this slot");

		mpRenderGraph->RemoveResourceDenpendency(mInResources[slot]->GetRenderTarget());
		mInResources[slot].reset();
		mResourceValidFlag &= UInt32(~(1 << slot));
	}

	void RenderGraphNode::DetachResource(const RenderGraphInReousrce& resource)
	{
		for (UInt32 i = 0; i < mInResources.size(); ++i)
		{
			if (mInResources[i] == resource)
			{
				mpRenderGraph->RemoveResourceDenpendency(mInResources[i]->GetRenderTarget());
				mInResources[i].reset();
				mResourceValidFlag &= UInt32(~(1 << i));
				break;
			}
		}
		SG_LOG_WARN("Failed to found resource in this Node!");
	}

	void RenderGraphNode::ClearResources()
	{
		for (auto& resource : mInResources)
		{
			mpRenderGraph->RemoveResourceDenpendency(resource->GetRenderTarget());
			resource.reset();
		}
		mResourceValidFlag = 0;
	}

	void RenderGraphNode::ResetFrameBuffer(Size frameBufferHash)
	{
		mpRenderGraph->ResetFrameBuffer(this, frameBufferHash);
	}

	bool RenderGraphNode::HaveValidResource() const
	{
		return !(mResourceValidFlag == 0);
	}

	//VkClearValue RenderGraphNode::ToVkClearValue(const InResourceType& resource)
	//{
	//	VkClearValue clearValue = {};
	//	auto& cv = resource->GetClearValue();
	//	if (!resource->GetRenderTarget()->IsDepth())
	//		clearValue.color = { { cv.color.float32[0], cv.color.float32[1], cv.color.float32[2], cv.color.float32[3] } };
	//	else
	//		clearValue.depthStencil = { cv.depthStencil.depth, cv.depthStencil.stencil };
	//	return clearValue;
	//}

}