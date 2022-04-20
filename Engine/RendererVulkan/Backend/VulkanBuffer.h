#pragma once

#include "Defs/Defs.h"
#include "Render/Buffer.h"

#include "VulkanConfig.h"
#include "VulkanContext.h"

#include "volk.h"

#include "vma/vk_mem_alloc.h"

namespace SG
{

	class VulkanBuffer
	{
	public:
		VulkanBuffer(VulkanContext& c, const BufferCreateDesc& CI);
		~VulkanBuffer();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanBuffer);

		bool   UploadData(const void* pData);
		bool   UploadData(const void* pData, UInt32 size, UInt32 offset);
		UInt32 Size() const { return size; }
		static VulkanBuffer* Create(VulkanContext& context, const BufferCreateDesc& CI);

		void* GetMappedMemory() const { return pMappedMemory; }

		template <typename DataType>
		DataType* GetMappedMemoryAs() const { return reinterpret_cast<DataType*>(pMappedMemory); }

		template <typename DataType>
		DataType* MapMemory();
		void      UnmapMemory();
	private:
		friend class VulkanCommandBuffer;
		friend class VulkanDescriptorDataBinder;

		VulkanContext& context;

		VmaAllocation  vmaAllocation;
		VkBuffer       buffer;
		UInt32         size;
		EBufferType    type;

		void*          pMappedMemory = nullptr;
	};

	template <typename DataType>
	DataType* VulkanBuffer::MapMemory()
	{
		//if (bLocal) // device local in GPU
		//{
		//	SG_LOG_WARN("Try to upload data to device local memory!");
		//	return false;
		//}

		DataType* pMappedMemory = nullptr;
		VK_CHECK(vmaMapMemory(context.vmaAllocator, vmaAllocation, (void**)&pMappedMemory),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		return pMappedMemory;
	}

}