#include "StdAfx.h"
#include "FrameBufferVk.h"

#include "Common/System/ILog.h"

#include "Common/Render/Renderer.h"
#include "RendererVulkan/Renderer/RendererVk.h"
#include "RendererVulkan/RenderContext/RenderContextVk.h"
#include "Common/Render/Pipeline.h"
#include "RendererVulkan/Pipeline/PipelineVk.h"
#include "RendererVulkan/SwapChain/SwapChainVk.h"
#include "Common/Render/Texture.h"

#include "Common/Platform/Window.h"

namespace SG
{

	FrameBufferVk::FrameBufferVk(Renderer* pRenderer)
		:mpRenderer(pRenderer)
	{
		mBuffers.resize(SG_SWAPCHAIN_IMAGE_COUNT);
		mSwapChainImagaViews.resize(SG_SWAPCHAIN_IMAGE_COUNT);

		Window* mainWindow = CSystem::GetInstance()->GetOS()->GetMainWindow();

		RenderPass* pRenderPass = new RenderPassVk(mpRenderer);

		SwapChain* pSwapChain = mpRenderer->GetSwapChain();
		Texture* ppSwapChainRt[SG_SWAPCHAIN_IMAGE_COUNT] = {};
		for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
			ppSwapChainRt[i] = pSwapChain->GetTexture(i);

		for (Size i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++) 
		{
			VkImageView attachments[] = {
				(VkImageView)ppSwapChainRt[i]->GetImageView()
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = (VkRenderPass)pRenderPass->GetNativeHandle();
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = pSwapChain->GetExtent().width;
			framebufferInfo.height = pSwapChain->GetExtent().height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), 
				&framebufferInfo, nullptr, &mBuffers[i]) != VK_SUCCESS) 
			{
				SG_LOG_ERROR("Failed to create frame buffer %d", i);
				SG_ASSERT(false);
			}
		}

		delete pRenderPass;
	}

	FrameBufferVk::~FrameBufferVk()
	{
		for (Size i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
		{
			vkDestroyFramebuffer((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mBuffers[i], nullptr);
			vkDestroyImageView((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mSwapChainImagaViews[i], nullptr);
		}
	}

}