#include "StdAfx.h"
#include "SwapChainVk.h"

#include "Common/System/ILog.h"
#include "RendererVulkan/Renderer/RendererVk.h"
#include "RendererVulkan/Queue/QueueVk.h"
#include "RendererVulkan/RenderContext/RenderContextVk.h"
#include "RendererVulkan/Texture/TextureVk.h"

#include "Common/Stl/vector.h"
#include <EASTL/algorithm.h>
#include <EASTL/utility.h>

namespace SG
{

	static UInt32 gImageFormatToVkMap[(UInt32)EImageFormat::MAX_COUNT] = {
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R4G4_UNORM_PACK8,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R8_UNORM,
		VK_FORMAT_R8G8_UNORM,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R8G8B8_UNORM,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_R8_SNORM,
		VK_FORMAT_R8G8_SNORM,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R8G8B8_SNORM,
		VK_FORMAT_R8G8B8A8_SNORM,
		VK_FORMAT_R8_UINT,
		VK_FORMAT_R8G8_UINT,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R8G8B8_UINT,
		VK_FORMAT_R8G8B8A8_UINT,
		VK_FORMAT_R8_SINT,
		VK_FORMAT_R8G8_SINT,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R8G8B8_SINT,
		VK_FORMAT_R8G8B8A8_SINT,
		VK_FORMAT_R8_SRGB,
		VK_FORMAT_R8G8_SRGB,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R8G8B8_SRGB,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_FORMAT_R16G16_UNORM,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R16G16_SNORM,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_R16G16_UINT,
		VK_FORMAT_R16G16_SINT,
		VK_FORMAT_R16G16_SFLOAT,
		VK_FORMAT_UNDEFINED,
		VK_FORMAT_A2R10G10B10_UNORM_PACK32,
		VK_FORMAT_A2R10G10B10_UINT_PACK32,
		VK_FORMAT_A2R10G10B10_SNORM_PACK32,
		VK_FORMAT_A2R10G10B10_SINT_PACK32,
		VK_FORMAT_R32G32_UINT,
		VK_FORMAT_R32G32_SINT,
		VK_FORMAT_R32G32_SFLOAT,
		VK_FORMAT_R32G32B32_UINT,
		VK_FORMAT_R32G32B32_SINT,
		VK_FORMAT_R32G32B32_SFLOAT,
		VK_FORMAT_R32G32B32A32_UINT,
		VK_FORMAT_R32G32B32A32_SINT,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_FORMAT_R64_UINT,
		VK_FORMAT_R64_SINT,
		VK_FORMAT_R64_SFLOAT,
		VK_FORMAT_R64G64_UINT,
		VK_FORMAT_R64G64_SINT,
		VK_FORMAT_R64G64_SFLOAT,
		VK_FORMAT_R64G64B64_UINT,
		VK_FORMAT_R64G64B64_SINT,
		VK_FORMAT_R64G64B64_SFLOAT,
		VK_FORMAT_R64G64B64A64_UINT,
		VK_FORMAT_R64G64B64A64_SINT,
		VK_FORMAT_R64G64B64A64_SFLOAT,
		VK_FORMAT_D16_UNORM,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_UNDEFINED,
	};

	static VkPresentModeKHR ToVkPresentMode(EPresentMode pmode)
	{
		switch (pmode)
		{
			case SG::EPresentMode::eImmediate:    return VK_PRESENT_MODE_IMMEDIATE_KHR;
			case SG::EPresentMode::eFIFO:         return VK_PRESENT_MODE_FIFO_KHR;
			case SG::EPresentMode::eFIFO_Relaxed: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			case SG::EPresentMode::eMailbox:      return VK_PRESENT_MODE_MAILBOX_KHR;
			case SG::EPresentMode::MAX_COUNT:
			default:
				SG_LOG_ERROR("Unsupported present mode");
				return VK_PRESENT_MODE_MAX_ENUM_KHR;
		}
	}

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
			if (format.format == gImageFormatToVkMap[(UInt32)mFormat])
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
		if (capabilities.currentExtent.width != UINT32_MAX) 
			extent = capabilities.currentExtent;
		else
		{
			extent.width =  eastl::clamp(extent.width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
			extent.height = eastl::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		UInt32 imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = presentSurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = (VkFormat)gImageFormatToVkMap[(UInt32)format];
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

		UInt32 imageCnt = SG_SWAPCHAIN_IMAGE_COUNT;
		vector<VkImage> mVkImages(imageCnt);
		mImages.resize(imageCnt);
		vkGetSwapchainImagesKHR((VkDevice)pRenderer->GetRenderContext()->GetLogicalDeviceHandle(),
			mHandle, &imageCnt, mVkImages.data());

		for (UInt32 i = 0; i < imageCnt; i++)
		{
			mImages[i] = new TextureVk(mpRenderer, mResolution, true);
			mImages[i]->mHandle = mVkImages[i];
		}
	}

	SwapChainVk::~SwapChainVk()
	{
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

}