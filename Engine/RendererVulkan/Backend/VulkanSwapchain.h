#pragma once

#include "Base/BasicTypes.h"
#include "RendererVulkan/Config.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"

namespace SG
{

	struct VulkanQueue;

	struct VulkanRenderTarget
	{
		VkImage               image;
		VkImageView           imageView;
		VkDeviceMemory        memory;
		VkFormat              format;
		VkImageType           type;
		VkSampleCountFlagBits sample;
		VkImageUsageFlagBits  usage;

		UInt32         width;
		UInt32         height;
		UInt32         depth;
		UInt32         array;
	};

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VkInstance instance);
		~VulkanSwapchain();

		VkSwapchainKHR       swapchain = VK_NULL_HANDLE;
		UInt32               imageCount;
		vector<VkImage>      images;
		vector<VkImageView>  imageViews;

		void BindDevice(VkPhysicalDevice physicalDevice, VkDevice device);

		bool CreateOrRecreate(UInt32 width, UInt32 height, bool vsync = false);
		void Destroy();

		bool CreateSurface();
		bool CheckSurfacePresentable(VulkanQueue queue);
	private:
		VkInstance       mInstance;
		VkPhysicalDevice mPhysicalDevice;
		VkDevice	     mLogicalDevice;
		bool             bSwapchainAdequate = false;

		VkSurfaceKHR     mPresentSurface;
		VkFormat         mFormat;
		VkColorSpaceKHR  mColorSpace;

		//vector<VulkanRenderTarget> pRt;
	};

}