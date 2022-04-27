#pragma once

#include "Base/BasicTypes.h"
#include "Render/Queue.h"
#include "Render/Shader/Shader.h"
#include "Render/Pipeline.h"
#include "Render/SwapChain.h"
#include "Render/Buffer.h"

#include "RendererVulkan/Config.h"
#include "volk.h"

#include <Stl/vector.h>
#include <Stl/string.h>
#include "eastl/list.h"

namespace SG
{

	class VulkanBuffer;
	class VulkanCommandPool;
	class VulkanRenderTarget;
	class VulkanRenderContext;
	class VulkanQueue;

	//! @brief All the device relative data are stored in here.
	//! Should be initialized by VulkanInstace.
	class VulkanDevice
	{
	public:
		VulkanDevice(VkPhysicalDevice& device);
		~VulkanDevice();

		VkDevice logicalDevice = VK_NULL_HANDLE;

		vector<VkExtensionProperties>   supportedExtensions;
		vector<VkQueueFamilyProperties> queueFamilyProperties;

		VkPhysicalDeviceLimits          physicalDeviceLimits;
		VkPhysicalDeviceFeatures        physicalDeviceFeatures;

		struct
		{
			UInt32 graphics;
			UInt32 compute;
			UInt32 transfer;
		} queueFamilyIndices;

		void WaitIdle() const;

		VulkanQueue* GetQueue(EQueueType type) const;

		VkDescriptorSet  AllocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool);
		void             FreeDescriptorSet(VkDescriptorSet set, VkDescriptorPool pool);

		bool SupportExtension(const string& extension);
		UInt32 GetMemoryType(UInt32 typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
	private:
		VkPhysicalDevice& physicalDevice;

		//! @brief Fetch all the queue family indices and create a logical device.
		bool CreateLogicalDevice(void* pNext);
		void DestroyLogicalDevice();

		int FetchQueueFamilyIndicies(VkQueueFlagBits type);
	private:
		mutable eastl::list<VulkanQueue> mQueues;
	};

}