#include "StdAfx.h"
#include "VulkanBuffer.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

#include "VulkanContext.h"

#include "RendererVulkan/Utils/VkConvert.h"

namespace SG
{

	VulkanBuffer::VulkanBuffer(VulkanContext& c, const BufferCreateDesc& CI)
		:context(c)
	{
		SG_ASSERT(CI.memoryUsage != EGPUMemoryUsage::eInvalid);

		if (CI.type == EBufferType::efUniform)
			size = MinValueAlignTo(CI.bufferSize, static_cast<UInt32>(context.device.physicalDeviceLimits.minUniformBufferOffsetAlignment));
		else
			size = CI.bufferSize;
		type = CI.type;
		memoryUsage = CI.memoryUsage;
		memoryFlag = CI.memoryFlag;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size  = size;
		bufferInfo.usage = ToVkBufferUsage(type);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		VmaAllocationCreateInfo vmaAllocateCI = { 0 };
		vmaAllocateCI.usage = ToVmaMemoryUsage(CI.memoryUsage);
		vmaAllocateCI.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
		if (SG_HAS_ENUM_FLAG(CI.memoryFlag, EGPUMemoryFlag::efDedicated_Memory))
			vmaAllocateCI.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		if (SG_HAS_ENUM_FLAG(CI.memoryFlag, EGPUMemoryFlag::efPersistent_Map))
			vmaAllocateCI.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo allocInfo = {};
		VK_CHECK(vmaCreateBuffer(context.vmaAllocator, &bufferInfo, &vmaAllocateCI, &buffer, &vmaAllocation, &allocInfo),
			SG_LOG_ERROR("Failed to create vulkan buffer!"););

		if (SG_HAS_ENUM_FLAG(CI.memoryFlag, EGPUMemoryFlag::efPersistent_Map))
			pMappedMemory = allocInfo.pMappedData;
#else
		VK_CHECK(vkCreateBuffer(context.device.logicalDevice, &bufferInfo, nullptr, &buffer),
			SG_LOG_ERROR("Failed to create vulkan buffer!"););

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(context.device.logicalDevice, buffer, &memReqs);
		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAlloc.allocationSize = memReqs.size;

		sizeInByteGPU = static_cast<UInt32>(memReqs.size);
		alignment     = static_cast<UInt32>(memReqs.alignment);

		if (bLocal)
			memAlloc.memoryTypeIndex = context.device.GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		else
			memAlloc.memoryTypeIndex = context.device.GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK(vkAllocateMemory(context.device.logicalDevice, &memAlloc, nullptr, &memory),
			SG_LOG_ERROR("Failed to alloc memory for buffer!"););

		// TODO: support memory offset.
		VK_CHECK(vkBindBufferMemory(context.device.logicalDevice, buffer, memory, 0),
			SG_LOG_ERROR("Failed to bind vulkan memory to buffer!"););
#endif
	}

	VulkanBuffer::~VulkanBuffer()
	{
#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		vmaDestroyBuffer(context.vmaAllocator, buffer, vmaAllocation);
#else
		vkFreeMemory(context.device.logicalDevice, memory, nullptr);
		vkDestroyBuffer(context.device.logicalDevice, buffer, nullptr);
#endif
	}

	bool VulkanBuffer::UploadData(const void* pData)
	{
		if (!IsHostVisible(memoryUsage))
		{
			SG_LOG_WARN("Try to upload data to device local memory! Please use sub-buffer to upload data.");
			return false;
		}

		if (SG_HAS_ENUM_FLAG(memoryFlag, EGPUMemoryFlag::efPersistent_Map))
		{
			memcpy(pMappedMemory, pData, size);
			return true;
		}

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		UInt8* pUpload = nullptr;
		VK_CHECK(vmaMapMemory(context.vmaAllocator, vmaAllocation, (void**)&pUpload),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		memcpy(pUpload, pData, size);
		vmaUnmapMemory(context.vmaAllocator, vmaAllocation);
		return true;
#else
		UInt8* pUpload = nullptr;
		VK_CHECK(vkMapMemory(context.device.logicalDevice, memory, 0, size, 0, (void**)&pUpload),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		memcpy(pUpload, pData, size);
		vkUnmapMemory(context.device.logicalDevice, memory);
		return true;
#endif
	}

	bool VulkanBuffer::UploadData(const void* pData, UInt32 size, UInt32 offset)
	{
		if (!IsHostVisible(memoryUsage))
		{
			SG_LOG_WARN("Try to upload data to device local memory! Please use sub-buffer to upload data.");
			return false;
		}

		if (SG_HAS_ENUM_FLAG(memoryFlag, EGPUMemoryFlag::efPersistent_Map))
		{
			auto* pUpload = reinterpret_cast<UInt8*>(pMappedMemory) + offset;
			memcpy(pUpload, pData, size);
			return true;
		}

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		UInt8* pUpload = nullptr;
		VK_CHECK(vmaMapMemory(context.vmaAllocator, vmaAllocation, (void**)&pUpload),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		pUpload += offset;
		memcpy(pUpload, pData, size);
		vmaUnmapMemory(context.vmaAllocator, vmaAllocation);
		return true;
#else
		UInt8* pUpload = nullptr;
		VK_CHECK(vkMapMemory(context.device.logicalDevice, memory, 0, size, 0, (void**)&pUpload),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		pUpload += offset;
		memcpy(pUpload, pData, size);
		vkUnmapMemory(context.device.logicalDevice, memory);
		return true;
#endif
	}

	UInt32 VulkanBuffer::SizeGPU() const
	{
#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		if (vmaAllocation)
		{
			VmaAllocationInfo info = {};
			vmaGetAllocationInfo(context.vmaAllocator, vmaAllocation, &info);
			return static_cast<UInt32>(info.size);
		}
#endif
		return 0;
	}

	VulkanBuffer* VulkanBuffer::Create(VulkanContext& c, const BufferCreateDesc& CI)
	{
		return New(VulkanBuffer, c, CI);
	}

	void VulkanBuffer::UnmapMemory()
	{
		if (SG_HAS_ENUM_FLAG(memoryFlag, EGPUMemoryFlag::efPersistent_Map)) // do nothing, since the memory is persitently mapping to CPU memory.
			return;

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		vmaUnmapMemory(context.vmaAllocator, vmaAllocation);
#else
		vkUnmapMemory(context.device.logicalDevice, memory);
#endif
	}

}