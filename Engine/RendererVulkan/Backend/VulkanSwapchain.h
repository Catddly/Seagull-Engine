#pragma once

#include <vulkan/vulkan_core.h>

namespace SG
{
	
	struct VulkanQueue;

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		~VulkanSwapchain();

		VkSwapchainKHR mHandle;

		bool CreateSurface(VulkanQueue graphicQueue);
	private:
		VkInstance       mInstance;
		VkPhysicalDevice mPhysicalDevice;
		VkDevice	     mLogicalDevice;
		bool             bSwapchainAdequate = false;

		VkSurfaceKHR     mPresentSurface;
	};

}