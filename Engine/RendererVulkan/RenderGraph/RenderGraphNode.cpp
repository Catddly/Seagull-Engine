#include "StdAfx.h"
#include "RenderGraphNode.h"

#include "System/Logger.h"

#include <eastl/algorithm.h>

namespace SG
{

	void RenderGraphNode::AttachResource(RenderGraphInReousrce resource)
	{
		mInResources.push_back(resource);
	}

	bool RenderGraphNode::ReplaceResource(RenderGraphInReousrce oldResource, RenderGraphInReousrce newResource)
	{
		for (UInt32 i = 0; i < mInResources.size(); ++i)
		{
			if (mInResources[i] == oldResource)
			{
				mInResources[i] = newResource;
				return true;
			}
		}
		return false;
	}

	void RenderGraphNode::ReplaceOrAttachResource(RenderGraphInReousrce oldResource, RenderGraphInReousrce newResource)
	{
		if (!ReplaceResource(oldResource, newResource))
			AttachResource(newResource);
	}

	void RenderGraphNode::DetachResource(RenderGraphInReousrce resource)
	{
		auto* pNode = eastl::find(mInResources.begin(), mInResources.end(), resource);
		if (pNode != mInResources.end())
			mInResources.erase(pNode);
		else
			SG_LOG_WARN("Try to detach an unexist resource in render graph!");
	}

	void RenderGraphNode::ClearResources()
	{
		mInResources.clear();
	}

}