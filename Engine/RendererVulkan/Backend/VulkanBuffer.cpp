#include "StdAfx.h"
#include "VulkanBuffer.h"

#include "System/ILogger.h"

#include "VulkanInstance.h"

namespace SG
{

	bool VulkanBuffer::UploadData(void* pData)
	{
		void* pUpload = nullptr;
		VK_CHECK(vkMapMemory(device, memory, 0, totalSizeInByte, 0, &pUpload), SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		memcpy(pUpload, pData, totalSizeInByte);
		vkUnmapMemory(device, memory);
		return true;
	}

	bool VulkanBuffer::UpdateDescriptor()
	{
		VkWriteDescriptorSet writeDescriptorSet = {};

		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &descriptor;
		// Binds this uniform buffer to binding point 0
		writeDescriptorSet.dstBinding = 0;

		vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
		return true;
	}

}