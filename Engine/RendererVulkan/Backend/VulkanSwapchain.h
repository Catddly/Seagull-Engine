#pragma once

#include "RendererVulkan/Config.h"

#include <vulkan/vulkan_core.h>

namespace SG
{
	
	struct VulkanQueue;

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VkInstance instance);
		~VulkanSwapchain();

		VkSwapchainKHR mHandle;

		void BindDevice(VkPhysicalDevice physicalDevice, VkDevice device);

		bool CreateSurface();
		void DestroySurface();
		bool CheckSurfacePresentable(VulkanQueue queue);
	private:
		VkInstance       mInstance;
		VkPhysicalDevice mPhysicalDevice;
		VkDevice	     mLogicalDevice;
		bool             bSwapchainAdequate = false;

		VkSurfaceKHR     mPresentSurface;
	};

}