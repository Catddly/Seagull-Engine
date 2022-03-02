#include "StdAfx.h"
#include "RenderGraphDependency.h"

#include "System/Logger.h"

namespace SG
{

	void RGResourceStatusKeeper::AddResourceDenpendency(VulkanRenderTarget* pRenderTarget, EResourceBarrier srcStatus, EResourceBarrier dstStatus)
	{
		if (mDenpendencies.find(pRenderTarget) != mDenpendencies.end())
		{
			SG_LOG_WARN("Already add dependency to this resource!");
			return;
		}
		mDenpendencies[pRenderTarget] = { srcStatus, dstStatus };

		// init current resource status
		mCurrResourcesStaus[pRenderTarget] = srcStatus;
	}

	RGResourceDenpendency RGResourceStatusKeeper::GetResourceNextStatus(VulkanRenderTarget* pRenderTarget)
	{
		if (mDenpendencies.find(pRenderTarget) == mDenpendencies.end())
		{
			SG_LOG_DEBUG("This resource have no dependency added to it yet!");
			SG_ASSERT(false);
		}

		auto& dependencyNode = mDenpendencies[pRenderTarget];
		auto& currDependency = mCurrResourcesStaus[pRenderTarget];
		if (currDependency == dependencyNode.srcStatus)
		{
			currDependency = dependencyNode.dstStatus;
			return { dependencyNode.srcStatus, dependencyNode.dstStatus };
		}
		else
		{
			return { currDependency, dependencyNode.dstStatus };
		}

		++mCurrNodeIndex;
	}

	void RGResourceStatusKeeper::Reset()
	{
		for (auto& node : mCurrResourcesStaus) // reset all to the srcStatus
			node.second = mDenpendencies[node.first].srcStatus;
		mCurrNodeIndex = 0;
	}

	void RGResourceStatusKeeper::Clear()
	{
		mDenpendencies.clear();
		mCurrResourcesStaus.clear();
		mCurrNodeIndex = 0;
	}

}