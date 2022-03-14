#include "StdAfx.h"
#include "VulkanBuffer.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

#include "VulkanConfig.h"
#include "RendererVulkan/Utils/VkConvert.h"

namespace SG
{

	VulkanBuffer::VulkanBuffer(VulkanDevice& d, const BufferCreateDesc& CI, bool local)
		:device(d), bLocal(local)
	{
		if (CI.type == EBufferType::efUniform)
			sizeInByteCPU = MinValueAlignTo(CI.totalSizeInByte, device.physicalDeviceLimits.minUniformBufferOffsetAlignment);
		else
			sizeInByteCPU = CI.totalSizeInByte;
		type = CI.type;

		VkMemoryRequirements memReqs;
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size  = sizeInByteCPU;
		bufferInfo.usage = ToVkBufferUsage(type);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK(vkCreateBuffer(device.logicalDevice, &bufferInfo, nullptr, &buffer),
			SG_LOG_ERROR("Failed to create vulkan buffer!"););

		vkGetBufferMemoryRequirements(device.logicalDevice, buffer, &memReqs);
		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAlloc.allocationSize = memReqs.size;

		sizeInByteGPU = static_cast<UInt32>(memReqs.size);
		alignment     = static_cast<UInt32>(memReqs.alignment);

		if (bLocal)
			memAlloc.memoryTypeIndex = device.GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		else
			memAlloc.memoryTypeIndex = device.GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK(vkAllocateMemory(device.logicalDevice, &memAlloc, nullptr, &memory),
			SG_LOG_ERROR("Failed to alloc memory for buffer!"););

		// TODO: support memory offset.
		VK_CHECK(vkBindBufferMemory(device.logicalDevice, buffer, memory, 0),
			SG_LOG_ERROR("Failed to bind vulkan memory to buffer!"););
	}

	VulkanBuffer::~VulkanBuffer()
	{
		vkFreeMemory(device.logicalDevice, memory, nullptr);
		vkDestroyBuffer(device.logicalDevice, buffer, nullptr);
	}

	bool VulkanBuffer::UploadData(const void* pData)
	{
		if (bLocal) // device local in GPU
		{
			SG_LOG_WARN("Try to upload data to device local memory!");
			return false;
		}

		UInt8* pUpload = nullptr;
		VK_CHECK(vkMapMemory(device.logicalDevice, memory, 0, sizeInByteCPU, 0, (void**)&pUpload), 
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		memcpy(pUpload, pData, sizeInByteCPU);
		vkUnmapMemory(device.logicalDevice, memory);
		return true;
	}

	bool VulkanBuffer::UploadData(const void* pData, UInt32 size, UInt32 offset)
	{
		if (bLocal) // device local in GPU
		{
			SG_LOG_WARN("Try to upload data to device local memory!");
			return false;
		}

		UInt8* pUpload = nullptr;
		VK_CHECK(vkMapMemory(device.logicalDevice, memory, 0, sizeInByteCPU, 0, (void**)&pUpload),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		pUpload += offset;
		memcpy(pUpload, pData, size);
		vkUnmapMemory(device.logicalDevice, memory);
		return true;
	}

	SG::VulkanBuffer* VulkanBuffer::Create(VulkanDevice& device, const BufferCreateDesc& CI, bool bLocal)
	{
		return Memory::New<VulkanBuffer>(device, CI, bLocal);
	}

}