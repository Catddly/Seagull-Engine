#include "StdAfx.h"
#include "VulkanQueryPool.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

#include "VulkanConfig.h"
#include "VulkanDevice.h"
#include "RendererVulkan/Utils/VkConvert.h"

namespace SG
{

	VulkanQueryPool::VulkanQueryPool(VulkanDevice& d, EPipelineStageQueryType pipelineStageTypes)
		:device(d), bSleep(false)
	{
		VkQueryPoolCreateInfo queryPoolCI = {};
		queryPoolCI.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;

		UInt32 count = 0;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efInput_Assembly_Vertices)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efInput_Assembly_Primitives)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efVertex_Shader_Invocations)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efGeometry_Shader_Invocation)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efGeometry_Shader_Primitives)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efClipping_Invocations)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efClipping_Primitives)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efFragment_Shader_Invocations)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efTessellation_Control_Shader_Patches)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efTessellation_Evaluation_Shader_Invocations)) ++count;
		if (SG_HAS_ENUM_FLAG(pipelineStageTypes, EPipelineStageQueryType::efCompute_Shader_Invocations)) ++count;

		queryPoolCI.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
		queryPoolCI.pipelineStatistics = ToVkQueryPipelineStatisticsFlag(pipelineStageTypes);
		queryPoolCI.queryCount = count;
		results.resize(count);
		timePeriod = -1.0f;
		queryCount = 1;

		VK_CHECK(vkCreateQueryPool(device.logicalDevice, &queryPoolCI, nullptr, &queryPool),
			SG_LOG_ERROR("Failed to create vulkan query pool!"););
	}

	VulkanQueryPool::VulkanQueryPool(VulkanDevice& d, UInt32 maxQueryCount)
		:device(d), bSleep(false)
	{
		VkQueryPoolCreateInfo queryPoolCI = {};
		queryPoolCI.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;

		queryPoolCI.queryType = VK_QUERY_TYPE_TIMESTAMP;
		queryPoolCI.queryCount = queryCount = maxQueryCount;
		timePeriod = device.physicalDeviceLimits.timestampPeriod;
		results.resize(maxQueryCount);

		VK_CHECK(vkCreateQueryPool(device.logicalDevice, &queryPoolCI, nullptr, &queryPool),
			SG_LOG_ERROR("Failed to create vulkan query pool!"););
	}

	VulkanQueryPool::~VulkanQueryPool()
	{
		vkDestroyQueryPool(device.logicalDevice, queryPool, nullptr);
	}

	const vector<QueryResult>& VulkanQueryPool::GetQueryResult()
	{
		if (bSleep)
			return results;

		vkGetQueryPoolResults(device.logicalDevice, queryPool,
			0, queryCount,
			results.size() * sizeof(QueryResult), results.data(), sizeof(QueryResult),
			VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
		return results;
	}

	double VulkanQueryPool::GetTimeStampDurationMs(UInt32 begin, UInt32 end)
	{
		if (bSleep)
			return 0.0f;

		SG_ASSERT(timePeriod != -1.0f);
		return (results[end].result - results[begin].result) * (timePeriod / 1e6);
	}

	VulkanQueryPool* VulkanQueryPool::Create(VulkanDevice& d, ERenderQueryType type, EPipelineStageQueryType pipelineStageTypes)
	{
		SG_ASSERT(type == ERenderQueryType::ePipeline_Statistics);
		return Memory::New<VulkanQueryPool>(d, pipelineStageTypes);
	}

	VulkanQueryPool* VulkanQueryPool::Create(VulkanDevice& d, ERenderQueryType type, UInt32 maxQueryCount)
	{
		SG_ASSERT(type == ERenderQueryType::eTimeStamp);
		return Memory::New<VulkanQueryPool>(d, maxQueryCount);
	}

}