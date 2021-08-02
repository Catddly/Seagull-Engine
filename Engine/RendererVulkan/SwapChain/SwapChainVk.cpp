#include "StdAfx.h"
#include "SwapChainVk.h"

#include "Common/System/ILog.h"
#include "RendererVulkan/Renderer/RendererVk.h"
#include "RendererVulkan/Queue/QueueVk.h"
#include "RendererVulkan/RenderContext/RenderContextVk.h"
#include "RendererVulkan/Texture/TextureVk.h"

#include "RendererVulkan/Utils/VulkanConversion.h"

#include "Common/Stl/vector.h"
#include <EASTL/algorithm.h>
#include <EASTL/utility.h>

namespace SG
{

	SwapChainVk::SwapChainVk(EImageFormat format, EPresentMode presentMode, const Resolution& res, Renderer* pRenderer)
		:mFormat(format), mPresentMode(presentMode), mResolution(res), mpRenderer(pRenderer)
	{
		//mTextures.resize(SG_SWAPCHAIN_IMAGE_COUNT);

		auto physicalDevice = (VkPhysicalDevice)pRenderer->GetRenderContext()->GetPhysicalDeviceHandle();
		auto presentSurface = (VkSurfaceKHR)pRenderer->GetRenderContext()->GetRenderSurface();

		VkSurfaceCapabilitiesKHR capabilities = {};
		vector<VkSurfaceFormatKHR> formats;
		vector<VkPresentModeKHR> presentModes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, presentSurface, &capabilities);

		UInt32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentSurface, &formatCount, nullptr);
		if (formatCount != 0) 
		{
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentSurface, &formatCount, formats.data());
		}

		UInt32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentSurface, &presentModeCount, nullptr);
		if (presentModeCount != 0) 
		{
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentSurface, &presentModeCount, presentModes.data());
		}

		// if the swapchain can do presenting
		bool bIsSwapChainAdequate = false;
		bIsSwapChainAdequate = !formats.empty() && !presentModes.empty();
		if (!bIsSwapChainAdequate)
			SG_LOG_WARN("Unpresentable swapchain detected");

		bool bFormatSupported = false;
		VkColorSpaceKHR colorSpace = {};
		for (auto& format : formats)
		{
			if (format.format == ToVkImageFormat(mFormat))
			{
				bFormatSupported = true;
				colorSpace = format.colorSpace;
				break;
			}
		}
		if (!bFormatSupported)
			SG_LOG_WARN("Unsupported image format");

		bool bPresentModeSupported = false;
		for (auto& pm : presentModes)
		{
			if (pm == ToVkPresentMode(presentMode))
			{
				bPresentModeSupported = true;
				break;
			}
		}
		if (!bPresentModeSupported)
			SG_LOG_WARN("Unsupported image format");

		// check for the swapchain extent
		VkExtent2D extent = { mResolution.width, mResolution.height };
		extent.width  = eastl::clamp(mResolution.width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
		extent.height = eastl::clamp(mResolution.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		mExtent.width = extent.width;
		mExtent.height = extent.height;
		
		UInt32 imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = presentSurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = ToVkImageFormat(format);
		createInfo.imageColorSpace = colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto* graphicQueue = pRenderer->GetGraphicQueue();
		auto* presentQueue = pRenderer->GetPresentQueue();
		UInt32 queueFamilyIndices[] = { graphicQueue->GetQueueIndex(), presentQueue->GetQueueIndex() };
		if (graphicQueue != presentQueue)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else 
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = ToVkPresentMode(presentMode);
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), &createInfo, nullptr, &mHandle) != VK_SUCCESS)
			SG_LOG_ERROR("Failed to create swapchain");

		// create texture container for swapchain images.
		mImages.resize(SG_SWAPCHAIN_IMAGE_COUNT);
		for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
			mImages[i] = new TextureVk(mpRenderer, this, i);
	}

	SwapChainVk::~SwapChainVk()
	{
		for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
			delete mImages[i];
		vkDestroySwapchainKHR((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mHandle, nullptr);
	}

	SG::Texture* SwapChainVk::GetTexture(UInt32 index) const
	{
		if (index >= SG_SWAPCHAIN_IMAGE_COUNT)
			SG_LOG_ERROR("Required texture of swapchain had been out of the coundary");

		return mImages[index];
	}

	SG::Handle SwapChainVk::GetNativeHandle() const
	{
		return mHandle;
	}

	SG::Resolution SwapChainVk::GetExtent() const
	{
		return mExtent;
	}

	SG::EImageFormat SwapChainVk::GetColorFormat() const
	{
		return mFormat;
	}

}