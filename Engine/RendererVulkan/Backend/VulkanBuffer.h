#pragma once

#include "Defs/Defs.h"
#include "Render/Buffer.h"

#include "VulkanDevice.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	class VulkanBuffer
	{
	public:
		VulkanBuffer(VulkanDevice& d, const BufferCreateDesc& CI, bool bLocal);
		~VulkanBuffer();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanBuffer);

		bool   UploadData(const void* pData);
		bool   UploadData(const void* pData, UInt32 size, UInt32 offset);
		UInt32 SizeInByteCPU() const { return sizeInByteCPU; }
		UInt32 SizeInByteGPU() const { return sizeInByteGPU; }
		static VulkanBuffer* Create(VulkanDevice& device, const BufferCreateDesc& CI, bool bLocal);

		template <typename UploadType>
		bool UploadData(const void* pData, UInt32 size, UInt32 offset);
	private:
		friend class VulkanCommandBuffer;
		friend class VulkanDescriptorDataBinder;
		VulkanDevice&  device;

		VkDeviceMemory memory;
		VkBuffer       buffer;
		UInt32         sizeInByteCPU;
		UInt32         sizeInByteGPU;
		UInt32         alignment;
		EBufferType    type;
		bool           bLocal;
	};

	template <typename UploadType>
	bool VulkanBuffer::UploadData(const void* pData, UInt32 size, UInt32 offset)
	{
		if (bLocal) // device local in GPU
		{
			SG_LOG_WARN("Try to upload data to device local memory!");
			return false;
		}

		UploadType* pMappedMemory = nullptr;
		VK_CHECK(vkMapMemory(device.logicalDevice, memory, 0, sizeInByteCPU, 0, (void**)&pMappedMemory),
			SG_LOG_ERROR("Failed to map vulkan buffer!"); return false;);
		pMappedMemory += offset;
		memcpy(pMappedMemory, pData, size);
		vkUnmapMemory(device.logicalDevice, memory);
		return true;
	}

}