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
		UInt32 SizeInByteCPU() const { return sizeInByteCPU; }
		UInt32 SizeInByteGPU() const { return sizeInByteGPU; }
		static VulkanBuffer* Create(VulkanDevice& device, const BufferCreateDesc& CI, bool bLocal);
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

}