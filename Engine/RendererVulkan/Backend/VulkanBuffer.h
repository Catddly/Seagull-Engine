#pragma once

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

		bool UploadData(void* pData);

		VkBuffer& NativeHandle() { return buffer; }
		static VulkanBuffer* Create(VulkanDevice& device, const BufferCreateDesc& CI, bool bLocal);
	private:
		VulkanDevice&  device;

		VkDeviceMemory memory;
		VkBuffer       buffer;
		UInt32         totalSizeInByte;
		EBufferType    type;
	};

}