#pragma once

#include "RendererVulkan/Config.h"
#include "Base/BasicTypes.h"
#include "Render/Queue.h"
#include "Render/Shader.h"
#include "Render/Pipeline.h"
#include "Render/SwapChain.h"
#include "Render/Buffer.h"

#include <vulkan/vulkan_core.h>

#include <Stl/vector.h>
#include <Stl/string.h>

namespace SG
{

	class VulkanCommandBuffer;
	class VulkanRenderPass;
	class VulkanSemaphore;
	class VulkanFence;

	class VulkanQueue
	{
	public:
		bool SubmitCommands(VulkanCommandBuffer* pCmdBuf, VulkanSemaphore* pSignalSemaphore, VulkanSemaphore* pWaitSemaphore, VulkanFence* fence);
		void WaitIdle() const;
	private:
		friend class VulkanSwapchain;
		friend class VulkanDevice;

		UInt32         familyIndex;
		EQueueType     type = EQueueType::eNull;
		EQueuePriority priority = EQueuePriority::eNormal;
		VkQueue        handle = VK_NULL_HANDLE;
	};

	class VulkanBuffer;
	class VulkanCommandPool;
	class VulkanRenderTarget;
	class VulkanRenderContext;

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

		struct
		{
			UInt32 graphics;
			UInt32 compute;
			UInt32 transfer;
		} queueFamilyIndices;

		void WaitIdle() const;

		VkFramebuffer CreateFrameBuffer(VkRenderPass renderPass, VulkanRenderTarget* pColorRt, VulkanRenderTarget* pDepthRt);
		void DestroyFrameBuffer(VkFramebuffer frameBuffer);

		VulkanQueue GetQueue(EQueueType type) const;

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
	};

}