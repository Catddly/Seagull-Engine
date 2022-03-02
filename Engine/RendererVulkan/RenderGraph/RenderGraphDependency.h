#pragma once

#include "Render/ResourceBarriers.h"

#include "Stl/unordered_map.h"

namespace SG
{

	class VulkanRenderTarget;

	struct RGResourceDenpendency
	{
		EResourceBarrier srcStatus;
		EResourceBarrier dstStatus;
	};

	class RGResourceStatusKeeper
	{
	public:
		void AddResourceDenpendency(VulkanRenderTarget* pRenderTarget, EResourceBarrier srcStatus, EResourceBarrier dstStatus);

		RGResourceDenpendency GetResourceNextStatus(VulkanRenderTarget* pRenderTarget);
		void Reset();
		void Clear();
	private:
		unordered_map<VulkanRenderTarget*, RGResourceDenpendency> mDenpendencies;
		unordered_map<VulkanRenderTarget*, EResourceBarrier>      mCurrResourcesStaus;
		UInt32 mCurrNodeIndex = 0;
	};

}