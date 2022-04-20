#include "StdAfx.h"
#include "VulkanBuffer.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

#include "VulkanConfig.h"
#include "RendererVulkan/Utils/VkConvert.h"

#include "vma/vk_mem_alloc.h"

namespace SG
{

	VulkanBuffer::VulkanBuffer(VulkanContext& c, const BufferCreateDesc& CI)
		:context(c)
	{
		if (CI.type == EBufferType::efUniform)
			size = MinValueAlignTo(CI.bufferSize, static_cast<UInt32>(context.device.physicalDeviceLimits.minUniformBufferOffsetAlignment));
		else
			size = CI.bufferSize;
		type = CI.type;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = ToVkBufferUsage(type);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vmaAllocateCI = { 0 };
		vmaAllocateCI.usage = (VmaMemoryUsage)CI.memoryUsage;
		vmaAllocateCI.flags = 0;
		if (SG_HAS_ENUM_FLAG(CI.memoryFlag, EGPUMemoryFlag::efDedicated_Memory))
			vmaAllocateCI.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		if (SG_HAS_ENUM_FLAG(CI.memoryFlag, EGPUMemoryFlag::efPersistent_Map))
			vmaAllocateCI.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo allocInfo = {};
		VK_CHECK(vmaCreateBuffer(context.vmaAllocator, &bufferInfo, &vmaAllocateCI, &buffer, &vmaAllocation, &allocInfo),
			SG_LOG_ERROR("Failed to create vulkan buffer!"););

		pMappedMemory = allocInfo.pMappedData;
	}

	VulkanBuffer::~VulkanBuffer()
	{
		vmaDestroyBuffer(context.vmaAllocator, buffer, vmaAllocation);
	}

	bool VulkanBuffer::UploadData(const void* pData)
	{
		//if (bLocal) // device local in GPU
		//{
		//	SG_LOG_WARN("Try to upload data to device local memory!");
		//	return false;
		//}

		UInt8* pUpload = nullptr;
		VK_CHECK(vmaMapMemory(context.vmaAllocator, vmaAllocation, (void**)&pUpload),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		memcpy(pUpload, pData, size);
		vmaUnmapMemory(context.vmaAllocator, vmaAllocation);
		return true;
	}

	bool VulkanBuffer::UploadData(const void* pData, UInt32 size, UInt32 offset)
	{
		//if (bLocal) // device local in GPU
		//{
		//	SG_LOG_WARN("Try to upload data to device local memory!");
		//	return false;
		//}

		UInt8* pUpload = nullptr;
		VK_CHECK(vmaMapMemory(context.vmaAllocator, vmaAllocation, (void**)&pUpload),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		pUpload += offset;
		memcpy(pUpload, pData, size);
		vmaUnmapMemory(context.vmaAllocator, vmaAllocation);
		return true;
	}

	VulkanBuffer* VulkanBuffer::Create(VulkanContext& context, const BufferCreateDesc& CI)
	{
		return Memory::New<VulkanBuffer>(context, CI);
	}

	void VulkanBuffer::UnmapMemory()
	{
		vmaUnmapMemory(context.vmaAllocator, vmaAllocation);
	}

}