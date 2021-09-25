#pragma once

#include "RendererVulkan/Config.h"
#include "Base/BasicTypes.h"
#include "Render/Queue.h"

#include <vulkan/vulkan_core.h>

#include <Stl/vector.h>
#include <Stl/string.h>

namespace SG
{

	struct VulkanQueue
	{
		UInt32         familyIndex;
		EQueueType     type = EQueueType::eNull;
		EQueuePriority priority = EQueuePriority::eNormal;
		VkQueue        handle = VK_NULL_HANDLE;
	};

	struct VulkanRenderTarget;

	//! @brief All the device relative data are stored in here.
	//! Should be initialized by VulkanInstace.
	class VulkanDevice
	{
	public:
		VulkanDevice(VkPhysicalDevice device);
		~VulkanDevice();

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice         logicalDevice = VK_NULL_HANDLE;

		VkCommandPool    defaultCommandPool;

		vector<VkExtensionProperties>   supportedExtensions;
		vector<VkQueueFamilyProperties> queueFamilyProperties;
		struct
		{
			UInt32 graphics;
			UInt32 compute;
			UInt32 transfer;
		} queueFamilyIndices;

		//! @brief Fetch all the queue family indices and create a logical device.
		bool CreateLogicalDevice(void* pNext);
		void DestroyLogicalDevice();

		//! @brief Create a command pool.
		VkCommandPool CreateCommandPool(UInt32 queueFamilyIndices, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		bool AllocateCommandBuffers(vector<VkCommandBuffer>& pCommandBuffers);
		void FreeCommandBuffers(vector<VkCommandBuffer>& pCommandBuffers);

		bool CreateRenderTarget(VulkanRenderTarget* rt);
		void DestroyRenderTarget(VulkanRenderTarget* rt);

		VulkanQueue GetQueue(EQueueType type) const;

		bool SupportExtension(const string& extension);
	private:
		int FetchQueueFamilyIndicies(VkQueueFlagBits type);
		UInt32 GetMemoryType(UInt32 typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
	};

}