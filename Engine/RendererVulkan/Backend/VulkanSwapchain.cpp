#include "StdAfx.h"
#include "VulkanSwapchain.h"

#include "System/System.h"
#include "Platform/IOperatingSystem.h"
#include "Platform/Window.h"

#include "VulkanDevice.h"
#include "RendererVulkan/Utils/VkConvert.h"

#ifdef SG_PLATFORM_WINDOWS
#	include <vulkan/vulkan_win32.h>
#endif

namespace SG
{

	VulkanSwapchain::VulkanSwapchain(VkInstance instance)
		:mInstance(instance), mPhysicalDevice(VK_NULL_HANDLE), mLogicalDevice(VK_NULL_HANDLE)
	{
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
	}

	void VulkanSwapchain::BindDevice(VkPhysicalDevice physicalDevice, VkDevice device)
	{
		if (physicalDevice != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
		{
			mPhysicalDevice = physicalDevice;
			mLogicalDevice = device;

			// Get list of supported surface formats
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mPresentSurface, &formatCount, NULL);
			vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mPresentSurface, &formatCount, surfaceFormats.data());

			// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
			// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
			if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
			{
				mFormat = VK_FORMAT_B8G8R8A8_UNORM;
				mColorSpace = surfaceFormats[0].colorSpace;
			}
			else
			{
				// iterate over the list of available surface format and
				// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
				bool found_B8G8R8A8_UNORM = false;
				for (auto&& surfaceFormat : surfaceFormats)
				{
					if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
					{
						mFormat = surfaceFormat.format;
						mColorSpace = surfaceFormat.colorSpace;
						found_B8G8R8A8_UNORM = true;
						break;
					}
				}

				// in case VK_FORMAT_B8G8R8A8_UNORM is not available
				// select the first available color format
				if (!found_B8G8R8A8_UNORM)
				{
					mFormat = surfaceFormats[0].format;
					mColorSpace = surfaceFormats[0].colorSpace;
				}
			}

			bSwapchainAdequate = true;
		}
		else
		{
			mPhysicalDevice = physicalDevice;
			mLogicalDevice = device;

			bSwapchainAdequate = false;
		}
	}

	bool VulkanSwapchain::CreateOrRecreate(UInt32 width, UInt32 height, bool vsync)
	{
		// if there is an old swapchain, use it to ease up recreation.
		VkSwapchainKHR oldSwapchain = swapchain;

		VkSurfaceCapabilitiesKHR capabilities = {};
		vector<VkSurfaceFormatKHR> formats;
		vector<VkPresentModeKHR> presentModes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mPresentSurface, &capabilities);

		UInt32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mPresentSurface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mPresentSurface, &formatCount, formats.data());
		}

		UInt32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mPresentSurface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mPresentSurface, &presentModeCount, presentModes.data());
		}

		// if the swapchain can do presenting
		bool bIsSwapChainAdequate = false;
		bIsSwapChainAdequate = !formats.empty() && !presentModes.empty();
		if (!bIsSwapChainAdequate)
			SG_LOG_WARN("Unpresentable swapchain detected");

		// the VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// this mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// if v-sync is not requested, try to find a mailbox mode
		// it's the lowest latency non-tearing present mode available
		if (!vsync)
		{
			for (Size i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		VkSurfaceCapabilitiesKHR surfCaps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mPresentSurface, &surfCaps);

		// find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;
		if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			// We prefer a non-rotated transform
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			preTransform = surfCaps.currentTransform;
		}

		VkExtent2D swapchainExtent = {};
		// if width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (surfCaps.currentExtent.width == (UInt32)-1)
		{
			// if the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfCaps.currentExtent;
			width = surfCaps.currentExtent.width;
			height = surfCaps.currentExtent.height;
		}

		imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mPresentSurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = mFormat;
		createInfo.imageColorSpace = mColorSpace;
		createInfo.imageExtent = swapchainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = swapchainPresentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = oldSwapchain;

		if (vkCreateSwapchainKHR(mLogicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create swapchain");
			return false;
		}

		// clean up old resoureces
		if (oldSwapchain != VK_NULL_HANDLE)
		{
			for (auto& e : imageViews)
			{
				vkDestroyImageView(mLogicalDevice, e, nullptr);
			}
			vkDestroySwapchainKHR(mLogicalDevice, swapchain, nullptr);
		}

		vkGetSwapchainImagesKHR(mLogicalDevice, swapchain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(mLogicalDevice, swapchain, &imageCount, images.data());

		imageViews.resize(imageCount);
		pRt.resize(imageCount);
		for (UInt32 i = 0; i < imageCount; ++i)
		{
			VkImageViewCreateInfo colorAttachmentView = {};
			colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorAttachmentView.pNext = NULL;
			colorAttachmentView.format = mFormat;
			colorAttachmentView.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorAttachmentView.flags = 0;

			colorAttachmentView.image = images[i];

			vkCreateImageView(mLogicalDevice, &colorAttachmentView, nullptr, &imageViews[i]);

			pRt[i].array = 1;
			pRt[i].format = mFormat;
			pRt[i].width = swapchainExtent.width;
			pRt[i].height = swapchainExtent.height;
			pRt[i].depth = 1;
			pRt[i].image = images[i];
			pRt[i].imageView = imageViews[i];
			pRt[i].memory = VK_NULL_HANDLE;
			pRt[i].sample = VK_SAMPLE_COUNT_1_BIT;
			pRt[i].type = VK_IMAGE_TYPE_2D;
			pRt[i].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}

		return true;
	}

	void VulkanSwapchain::Destroy()
	{
		for (UInt32 i = 0; i < imageCount; ++i)
			vkDestroyImageView(mLogicalDevice, imageViews[i], nullptr);
		if (swapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(mLogicalDevice, swapchain, nullptr);
		if (mPresentSurface != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(mInstance, mPresentSurface, nullptr);
	}

	bool VulkanSwapchain::CreateSurface()
	{
#ifdef SG_PLATFORM_WINDOWS
		auto* pOS = SSystem()->GetOS();
		Window* mainWindow = pOS->GetMainWindow();

		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)mainWindow->GetNativeHandle();
		createInfo.hinstance = ::GetModuleHandle(NULL);
		
		if (vkCreateWin32SurfaceKHR(mInstance, &createInfo, nullptr, &mPresentSurface) != VK_SUCCESS)
			return false;
		return true;
#endif
	}

	bool VulkanSwapchain::CheckSurfacePresentable(VulkanQueue queue)
	{
		// check if the graphic queue can do the presentation job
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, queue.familyIndex, mPresentSurface, &presentSupport);
		if (!presentSupport)
		{
			SG_LOG_ERROR("Current physical device not support surface presentation");
			return false;
		}
		return true;
	}

}