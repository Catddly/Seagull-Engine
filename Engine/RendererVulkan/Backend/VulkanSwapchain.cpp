#include "StdAfx.h"
#include "VulkanSwapchain.h"

#include "System/System.h"
#include "Platform/OS.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSynchronizePrimitive.h"
#include "RendererVulkan/Utils/VkConvert.h"

#include "Memory/Memory.h"

#ifdef SG_PLATFORM_WINDOWS
#	include <vulkan/vulkan_win32.h>
#endif

namespace SG
{

	VulkanSwapchain::VulkanSwapchain(VulkanInstance& instance, VulkanDevice& device)
		:mInstance(instance), mDevice(device)
	{
		CreateSurface();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		DestroySurface();
	}

	bool VulkanSwapchain::CreateOrRecreate(UInt32 width, UInt32 height, bool vsync)
	{
		// if there is an old swapchain, use it to ease up recreation.
		VkSwapchainKHR oldSwapchain = swapchain;

		VkSurfaceCapabilitiesKHR capabilities = {};
		vector<VkSurfaceFormatKHR> formats;
		vector<VkPresentModeKHR> presentModes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mInstance.physicalDevice, mPresentSurface, &capabilities);

		UInt32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mInstance.physicalDevice, mPresentSurface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mInstance.physicalDevice, mPresentSurface, &formatCount, formats.data());
		}

		UInt32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mInstance.physicalDevice, mPresentSurface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mInstance.physicalDevice, mPresentSurface, &presentModeCount, presentModes.data());
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
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mInstance.physicalDevice, mPresentSurface, &surfCaps);

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

		if (vkCreateSwapchainKHR(mDevice.logicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create swapchain");
			return false;
		}

		// clean up old resources
		if (oldSwapchain != VK_NULL_HANDLE)
		{
			for (auto& e : imageViews)
				vkDestroyImageView(mDevice.logicalDevice, e, nullptr);
			vkDestroySwapchainKHR(mDevice.logicalDevice, oldSwapchain, nullptr);
			for (UInt32 i = 0; i < imageCount; ++i)
				Memory::Delete(mpRts[i]);
		}

		vkGetSwapchainImagesKHR(mDevice.logicalDevice, swapchain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(mDevice.logicalDevice, swapchain, &imageCount, images.data());

		imageViews.resize(imageCount);
		mpRts.resize(imageCount);
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

			vkCreateImageView(mDevice.logicalDevice, &colorAttachmentView, nullptr, &imageViews[i]);

			mpRts[i] = Memory::New<VulkanRenderTarget>(mDevice);
			mpRts[i]->width  = swapchainExtent.width;
			mpRts[i]->height = swapchainExtent.height;
			mpRts[i]->depth  = 1;
			mpRts[i]->array  = 1;
			mpRts[i]->mipLevel = 1;

			mpRts[i]->format = ToSGImageFormat(mFormat);
			mpRts[i]->sample = ToSGSampleCount(VK_SAMPLE_COUNT_1_BIT);
			mpRts[i]->type   = ToSGImageType(VK_IMAGE_TYPE_2D);
			mpRts[i]->usage  = ToSGImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

			mpRts[i]->image     = images[i];
			mpRts[i]->imageView = imageViews[i];
			mpRts[i]->memory    = 0;
			mpRts[i]->mbIsDepth = false;

			mpRts[i]->id = TextureIDAllocator::NewID();
		}

		return true;
	}

	void VulkanSwapchain::DestroySurface()
	{
		if (mPresentSurface != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(mInstance.instance, mPresentSurface, nullptr);
	}

	VulkanRenderTarget* VulkanSwapchain::GetRenderTarget(UInt32 index) const
	{
		SG_ASSERT(index >= 0 && index < imageCount);
		if (mpRts.empty())
			SG_LOG_WARN("Swapchain not created yet!");
		return mpRts[index];
	}

	void VulkanSwapchain::CleanUp()
	{
		for (UInt32 i = 0; i < imageCount; ++i)
			vkDestroyImageView(mDevice.logicalDevice, imageViews[i], nullptr);
		for (auto* ptr : mpRts)
			Memory::Delete(ptr);
		if (swapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(mDevice.logicalDevice, swapchain, nullptr);
	}

	bool VulkanSwapchain::AcquireNextImage(VulkanSemaphore* signalSemaphore, UInt32& imageIndex)
	{
		if (vkAcquireNextImageKHR(mDevice.logicalDevice, swapchain, UINT64_MAX, signalSemaphore->semaphore, VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS)
		{
			SG_LOG_WARN("Failed to acquire next image!");
			return false;
		}
		return true;
	}

	EImageState VulkanSwapchain::Present(VulkanQueue* queue, UInt32 imageIndex, VulkanSemaphore* signalSemaphore)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;
		if (signalSemaphore != nullptr)
		{
			presentInfo.pWaitSemaphores = &signalSemaphore->semaphore;
			presentInfo.waitSemaphoreCount = 1;
		}

		VkResult res = vkQueuePresentKHR(queue->handle, &presentInfo);
		if (res == VK_SUCCESS)
			return EImageState::eComplete;
		else if (res == VK_SUBOPTIMAL_KHR)
			return EImageState::eIncomplete;
		return EImageState::eFailure;
	}

	bool VulkanSwapchain::CreateSurface()
	{
#ifdef SG_PLATFORM_WINDOWS
		Window* mainWindow = OperatingSystem::GetMainWindow();

		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)mainWindow->GetNativeHandle();
		createInfo.hinstance = ::GetModuleHandle(NULL);
		
		if (vkCreateWin32SurfaceKHR(mInstance.instance, &createInfo, nullptr, &mPresentSurface) != VK_SUCCESS)
			return false;

		CheckSurfacePresentable(mDevice.queueFamilyIndices.graphics);

		// Get list of supported surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mInstance.physicalDevice, mPresentSurface, &formatCount, NULL);
		vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mInstance.physicalDevice, mPresentSurface, &formatCount, surfaceFormats.data());

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
			for (auto& surfaceFormat : surfaceFormats)
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

		return true;
#endif
	}

	bool VulkanSwapchain::CheckSurfacePresentable(UInt32 familyIndex)
	{
		// check if the graphic queue can do the presentation job
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(mInstance.physicalDevice, familyIndex, mPresentSurface, &presentSupport);
		if (!presentSupport)
		{
			SG_LOG_ERROR("Current physical device not support surface presentation");
			return false;
		}
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanRenderTarget
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanRenderTarget* VulkanRenderTarget::Create(VulkanDevice& d, const TextureCreateDesc& CI, bool isDepth)
	{
		return Memory::New<VulkanRenderTarget>(d, CI, isDepth);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanTexture
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanTexture::VulkanTexture(VulkanDevice& d, const TextureCreateDesc& CI, bool bLocal)
		:device(d)
	{
		if (!IsValidImageFormat(CI.format))
			SG_ASSERT(false);

		currLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		width    = CI.width;
		height   = CI.height;
		depth    = CI.depth;
		mipLevel = CI.mipLevel;
		array    = CI.array;

		format = CI.format;
		type   = CI.type;
		sample = CI.sample;
		usage  = CI.usage;

		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = ToVkImageType(type);
		imageCI.format = ToVkImageFormat(format);
		imageCI.extent = { width, height, depth };
		imageCI.mipLevels = mipLevel;
		imageCI.arrayLayers = array;
		imageCI.samples = ToVkSampleCount(sample);
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = ToVkImageUsage(usage);
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK(vkCreateImage(device.logicalDevice, &imageCI, nullptr, &image),
			SG_LOG_ERROR("Failed to create vulkan texture!"););

		VkMemoryRequirements memReqs = {};
		vkGetImageMemoryRequirements(device.logicalDevice, image, &memReqs);

		VkMemoryAllocateInfo memAllloc = {};
		memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllloc.allocationSize = memReqs.size;
		memAllloc.memoryTypeIndex = device.GetMemoryType(memReqs.memoryTypeBits, bLocal ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT :
			(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		vkAllocateMemory(device.logicalDevice, &memAllloc, nullptr, &memory);
		vkBindImageMemory(device.logicalDevice, image, memory, 0);

		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.viewType = ToVkImageViewType(imageCI.imageType, array);
		imageViewCI.image  = image;
		imageViewCI.format = imageCI.format;
		imageViewCI.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.levelCount = mipLevel;
		imageViewCI.subresourceRange.layerCount = array;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		if (usage == EImageUsage::efDepth_Stencil)
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

		VK_CHECK(vkCreateImageView(device.logicalDevice, &imageViewCI, nullptr, &imageView),
			SG_LOG_ERROR("Failed to create vulkan texture image view!"););

		id = TextureIDAllocator::NewID();
	}

	VulkanTexture::~VulkanTexture()
	{
		if (memory != 0)
		{
			vkDestroyImageView(device.logicalDevice, imageView, nullptr);
			vkDestroyImage(device.logicalDevice, image, nullptr);
			vkFreeMemory(device.logicalDevice, memory, nullptr);
		}
	}

	VulkanTexture* VulkanTexture::Create(VulkanDevice& d, const TextureCreateDesc& CI, bool bLocal)
	{
		if (!IsPowerOfTwo(CI.width) || !IsPowerOfTwo(CI.height))
		{
			SG_LOG_ERROR("width(height) of texture must be the power of 2!");
			return nullptr;
		}

		return Memory::New<VulkanTexture>(d, CI, bLocal);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanSampler
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanSampler::VulkanSampler(VulkanDevice& d, const SamplerCreateDesc& CI)
		: device(d)
	{
		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = ToVkFilterMode(CI.filterMode);
		samplerCI.minFilter = ToVkFilterMode(CI.filterMode);
		samplerCI.mipmapMode = ToVkMipmapMode(CI.filterMode);
		samplerCI.addressModeU = ToVkAddressMode(CI.addressMode);
		samplerCI.addressModeV = ToVkAddressMode(CI.addressMode);
		samplerCI.addressModeW = ToVkAddressMode(CI.addressMode);
		samplerCI.mipLodBias = CI.lodBias;
		samplerCI.compareOp = VK_COMPARE_OP_NEVER;
		samplerCI.minLod = CI.minLod;
		samplerCI.maxLod = CI.maxLod;
		// if this physical render device can use anisotropy
		if (device.physicalDeviceFeatures.samplerAnisotropy) 
		{
			samplerCI.maxAnisotropy = device.physicalDeviceLimits.maxSamplerAnisotropy;
			samplerCI.anisotropyEnable = VK_TRUE;
		}
		else 
		{
			samplerCI.maxAnisotropy = 1.0;
			samplerCI.anisotropyEnable = VK_FALSE;
		}
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VK_CHECK(vkCreateSampler(device.logicalDevice, &samplerCI, nullptr, &sampler),
			SG_LOG_ERROR("Failed to create vulkan sampler!"););
	}

	VulkanSampler::~VulkanSampler()
	{
		vkDestroySampler(device.logicalDevice, sampler, nullptr);
	}

	VulkanSampler* VulkanSampler::Create(VulkanDevice& d, const SamplerCreateDesc& CI)
	{
		return Memory::New<VulkanSampler>(d, CI);
	}

}