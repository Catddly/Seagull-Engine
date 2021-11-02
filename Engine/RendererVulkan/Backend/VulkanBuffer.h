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

		bool UploadData(void* pData);

		VkBuffer& NativeHandle() { return buffer; }
		UInt32    SizeInByte() const { return totalSizeInByte; }
		static VulkanBuffer* Create(VulkanDevice& device, const BufferCreateDesc& CI, bool bLocal);
	private:
		VulkanDevice&  device;

		VkDeviceMemory memory;
		VkBuffer       buffer;
		UInt32         totalSizeInByte;
		EBufferType    type;
	};

}