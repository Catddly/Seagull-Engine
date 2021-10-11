#include "StdAfx.h"
#include "VulkanBuffer.h"

#include "System/ILogger.h"

#include "VulkanInstance.h"

namespace SG
{

	bool VulkanBuffer::Upload(VkDevice device, void* pData)
	{
		void* pUpload = nullptr;
		VK_CHECK(vkMapMemory(device, memory, 0, sizeInByte, 0, &pUpload), SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		memcpy(pUpload, pData, sizeInByte);
		vkUnmapMemory(device, memory);
		return true;
	}

}