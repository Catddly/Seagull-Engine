#include "StdAfx.h"
#include "TextureVk.h"

#include "System/ILogger.h"

#include "Render/Renderer.h"
#include "RendererVulkan/Renderer/RendererVk.h"
#include "RendererVulkan/RenderContext/RenderContextVk.h"
#include "RendererVulkan/SwapChain/SwapChainVk.h"

namespace SG
{

	TextureVk::TextureVk(Renderer* pRenderer, const Resolution& res)
		:mResolution(res), mpRenderer(pRenderer), mbIsSwapchainImage(false)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = mImage;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
		// no swizzle
		createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_A;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(),
			&createInfo, nullptr, &mImageView) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create image view!");
			SG_ASSERT(false);
		}
	}

	TextureVk::TextureVk(Renderer* pRenderer, Old_SwapChain* pSwapChain, UInt32 index)
		:mpRenderer(pRenderer), mbIsSwapchainImage(true)
	{
		SG_ASSERT(index >= 0 && index < SG_SWAPCHAIN_IMAGE_COUNT);

		// fetch image from swapchain
		UInt32 imageCnt = SG_SWAPCHAIN_IMAGE_COUNT;
		VkImage vkImages[SG_SWAPCHAIN_IMAGE_COUNT] = { };
		vkGetSwapchainImagesKHR((VkDevice)pRenderer->GetRenderContext()->GetLogicalDeviceHandle(),
			(VkSwapchainKHR)pSwapChain->GetNativeHandle(), &imageCnt, vkImages);
		mImage = vkImages[index];

		// create image view for swapchain image
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = mImage;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
		// no swizzle
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(),
			&createInfo, nullptr, &mImageView) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create swapchain image view!");
			SG_ASSERT(false);
		}
	}

	TextureVk::~TextureVk()
	{
		vkDestroyImageView((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mImageView, nullptr);
		if (!mbIsSwapchainImage)
			vkDestroyImage((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mImage, nullptr);
	}

	Resolution TextureVk::GetResolution() const
	{
		return mResolution;
	}

	SG::Handle TextureVk::GetImage() const
	{
		return mImage;
	}

	SG::Handle TextureVk::GetImageView() const
	{
		return mImageView;
	}

}
