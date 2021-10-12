#pragma once

#include "Render/Buffer.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct VulkanBuffer
	{
		VkDevice       device; // TODO: remove it from buffer.
		VkDeviceMemory memory;
		VkBuffer       buffer;
		UInt32         totalSizeInByte;
		EBufferType    type;

		// descriptor for GPU to get the resource.
		VkDescriptorSetLayout  descriptorSetLayout;
		VkDescriptorSet        descriptorSet;
		VkDescriptorBufferInfo descriptor;

		bool UploadData(void* pData);
		bool UpdateDescriptor();
	};

}