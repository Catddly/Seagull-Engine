#include "StdAfx.h"
#include "RenderGraphNode.h"

#include "System/Logger.h"

#include <eastl/algorithm.h>

namespace SG
{

	RenderGraphNode::RenderGraphNode()
		:mResourceValidFlag(0)
	{
		//mInResources.resize(SG_MAX_RENDER_GRAPH_NODE_RESOURCE);
	}

	void RenderGraphNode::AttachResource(UInt32 slot, const RenderGraphInReousrce& resource)
	{
		SG_ASSERT(slot <= SG_MAX_RENDER_GRAPH_NODE_RESOURCE && "Exceed the maximun limit of render graph node resources.");
		mInResources[slot] = resource;
		mResourceValidFlag |= UInt32(1 << slot);
	}

	void RenderGraphNode::DetachResource(UInt32 slot)
	{
		SG_ASSERT(slot <= SG_MAX_RENDER_GRAPH_NODE_RESOURCE && "Exceed the maximun limit of render graph node resources.");
		mInResources[slot].reset();
		mResourceValidFlag &= UInt32(~(1 << slot));
	}

	void RenderGraphNode::DetachResource(const RenderGraphInReousrce& resource)
	{
		for (UInt32 i = 0; i < mInResources.size(); ++i)
		{
			if (mInResources[i] == resource)
			{
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
			resource.reset();
		mResourceValidFlag = 0;
	}

	bool RenderGraphNode::HaveValidResource() const
	{
		return !(mResourceValidFlag == 0);
	}

}