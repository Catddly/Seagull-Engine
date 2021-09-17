#pragma once

#include "RendererVulkan/Config.h"
#include "Render/SwapChain.h"

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

#ifdef SG_DEBUG
#	define SG_ENABLE_VK_VALIDATION_LAYER 1
#else
#	define SG_ENABLE_VK_VALIDATION_LAYER 0
#endif

	//! @brief All the singleton data in one render device.
	//! Should be initialized just once and its life cycle is the same
	//! as the render device. 
	class VulkanInstance
	{
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanInstance);
	public:
		VulkanInstance();
		~VulkanInstance();
	private:
		bool CreateVkInstance();
		void ValidateExtensions(vector<const char*>& extens, VkInstanceCreateInfo* info);
		void ValidateLayers(vector<const char*>& layers, VkInstanceCreateInfo* info);
		void PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager);
		void SetupDebugMessenger();

		bool SelectPhysicalDeviceAndCreateDevice();
	private:
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		VkDebugUtilsMessengerEXT mDebugLayer;
#endif
		VkInstance       mInstance = VK_NULL_HANDLE;
		VulkanDevice*    mDevice = nullptr;
		VulkanSwapchain* mSwapchain = nullptr;

		VulkanQueue     mGraphicQueue;
	};

}