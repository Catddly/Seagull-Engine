#include "StdAfx.h"
#include "VulkanTexture.h"

#include "Memory/Memory.h"

#include "VulkanConfig.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "RendererVulkan/Utils/VkConvert.h"

namespace SG
{

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
		samplerCI.mipmapMode = ToVkMipmapMode(CI.mipmapMode);
		samplerCI.addressModeU = ToVkAddressMode(CI.addressMode);
		samplerCI.addressModeV = ToVkAddressMode(CI.addressMode);
		samplerCI.addressModeW = ToVkAddressMode(CI.addressMode);
		samplerCI.mipLodBias = CI.lodBias;
		samplerCI.compareOp = VK_COMPARE_OP_NEVER;
		samplerCI.minLod = CI.minLod;
		samplerCI.maxLod = CI.maxLod;

		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		// if this physical render device can use anisotropy
		if (CI.enableAnisotropy && device.physicalDeviceFeatures.samplerAnisotropy)
		{
			samplerCI.maxAnisotropy = device.physicalDeviceLimits.maxSamplerAnisotropy;
			samplerCI.anisotropyEnable = VK_TRUE;
		}
		else
		{
			samplerCI.maxAnisotropy = 1.0;
			samplerCI.anisotropyEnable = VK_FALSE;
		}
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

		VK_CHECK(vkCreateSampler(device.logicalDevice, &samplerCI, nullptr, &sampler),
			SG_LOG_ERROR("Failed to create vulkan sampler!"););
	}

	VulkanSampler::~VulkanSampler()
	{
		vkDestroySampler(device.logicalDevice, sampler, nullptr);
	}

	VulkanSampler* VulkanSampler::Create(VulkanDevice& d, const SamplerCreateDesc& CI)
	{
		return New(VulkanSampler, d, CI);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanTexture
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	IDAllocator<UInt32> VulkanTexture::msIdAllocator;

	VulkanTexture::VulkanTexture(VulkanContext& c, const TextureCreateDesc& CI)
		:context(c)
	{
		if (!IsValidImageFormat(CI.format))
			SG_ASSERT(false);

		pUserData = CI.pUserData;

		width = CI.width;
		height = CI.height;
		depth = CI.depth;
		mipLevel = CI.mipLevel;
		array = CI.array;

		format = CI.format;
		type = CI.type;
		sample = CI.sample;
		usage = CI.usage;

		currLayouts.resize(mipLevel);
		for (auto& layout : currLayouts)
			layout = ToVkImageLayout(CI.initLayout);

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
		imageCI.initialLayout = ToVkImageLayout(CI.initLayout);
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (array == 6)
			imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		VmaAllocationCreateInfo vmaAllocationCI = {};
		vmaAllocationCI.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
		if (SG_HAS_ENUM_FLAG(CI.memoryFlag, EGPUMemoryFlag::efDedicated_Memory))
			vmaAllocationCI.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		 
		// for now, all textures are host invisible.
		vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VmaAllocationInfo allocInfo = {};
		VK_CHECK(vmaCreateImage(context.vmaAllocator, &imageCI, &vmaAllocationCI, &image, &vmaAllocation, &allocInfo),
			SG_LOG_ERROR("Failed to create vulkan texture!"););
#else
		VK_CHECK(vkCreateImage(context.device.logicalDevice, &imageCI, nullptr, &image),
			SG_LOG_ERROR("Failed to create vulkan texture!"););

		VkMemoryRequirements memReqs = {};
		vkGetImageMemoryRequirements(context.device.logicalDevice, image, &memReqs);

		VkMemoryAllocateInfo memAllloc = {};
		memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllloc.allocationSize = memReqs.size;
		memAllloc.memoryTypeIndex = context.device.GetMemoryType(memReqs.memoryTypeBits, bLocal ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT :
			(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		vkAllocateMemory(context.device.logicalDevice, &memAllloc, nullptr, &memory);
		vkBindImageMemory(context.device.logicalDevice, image, memory, 0);
#endif

		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.viewType = ToVkImageViewType(imageCI.imageType, array);
		imageViewCI.image = image;
		imageViewCI.format = imageCI.format;
		imageViewCI.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.levelCount = mipLevel;
		imageViewCI.subresourceRange.layerCount = array;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		if (format == EImageFormat::eUnorm_D24_uint_S8 || format == EImageFormat::eSfloat_D32_uint_S8)
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		else if (format == EImageFormat::eSfloat_D32 || format == EImageFormat::eUnorm_D16)
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		VK_CHECK(vkCreateImageView(context.device.logicalDevice, &imageViewCI, nullptr, &imageView),
			SG_LOG_ERROR("Failed to create vulkan texture image view!"););

		id = msIdAllocator.Allocate();
	}

	VulkanTexture::~VulkanTexture()
	{
#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		if (vmaAllocation)
		{
			vkDestroyImageView(context.device.logicalDevice, imageView, nullptr);
			vmaDestroyImage(context.vmaAllocator, image, vmaAllocation);
		}
#else
		if (memory != 0)
		{
			vkDestroyImageView(context.device.logicalDevice, imageView, nullptr);
			vkDestroyImage(context.device.logicalDevice, image, nullptr);
			vkFreeMemory(context.device.logicalDevice, memory, nullptr);
		}
#endif
		msIdAllocator.Restore(id);
	}

	VulkanTexture* VulkanTexture::Create(VulkanContext& c, const TextureCreateDesc& CI)
	{
		if (!IsPowerOfTwo(CI.width) || !IsPowerOfTwo(CI.height))
		{
			SG_LOG_ERROR("width(height) of texture must be the power of 2!");
			return nullptr;
		}

		return New(VulkanTexture, c, CI);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanRenderTarget
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanRenderTarget* VulkanRenderTarget::Create(VulkanContext& c, const TextureCreateDesc& CI, bool isDepth)
	{
		return New(VulkanRenderTarget, c, CI, isDepth);
	}

}