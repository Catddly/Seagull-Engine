#pragma once

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct VulkanBuffer
	{
		VkDeviceMemory memory;
		VkBuffer       buffer;
		UInt32         sizeInByte;

		bool Upload(VkDevice device, void* pData);
	};

}