#pragma once

#include "Render/Query.h"

#include "volk.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanDevice;

	class VulkanQueryPool
	{
	public:
		VulkanQueryPool(VulkanDevice& d, EPipelineStageQueryType pipelineStageTypes);
		VulkanQueryPool(VulkanDevice& d, UInt32 maxQueryCount);
		~VulkanQueryPool();

		bool IsSleep() const { return bSleep; }

		void Wake() { bSleep = false; }
		void Sleep() { bSleep = true; }

		float GetTimePeriod() const { return timePeriod; }
		eastl::pair<vector<QueryResult>, bool> GetQueryResult() const;
		//double GetTimeStampDurationMs(UInt32 begin, UInt32 end);

		static VulkanQueryPool* Create(VulkanDevice& d, ERenderQueryType type, EPipelineStageQueryType pipelineStageTypes);
		static VulkanQueryPool* Create(VulkanDevice& d, ERenderQueryType type, UInt32 maxQueryCount);
	private:
		friend class VulkanCommandBuffer;

		VulkanDevice& device;
		VkQueryPool queryPool;

		UInt32 queryCount;
		float  timePeriod;
		mutable vector<QueryResult> results;
		bool   bSleep;
	};

}