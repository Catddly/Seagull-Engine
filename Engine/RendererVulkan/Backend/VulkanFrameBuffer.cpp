#include "StdAfx.h"
#include "VulkanFrameBuffer.h"

#include "VulkanConfig.h"
#include "VulkanSwapchain.h"

#include "Memory/Memory.h"

#include "Stl/vector.h"

namespace SG
{

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanRenderPass
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	VulkanRenderPass::Builder& VulkanRenderPass::Builder::BindColorRenderTarget(VulkanRenderTarget* color, EResourceBarrier initStatus, EResourceBarrier dstStatus)
	{
		VkAttachmentDescription attachDesc = {};
		attachDesc.format  = color->format;
		attachDesc.samples = color->sample;
		// TODO: add load store mask
		attachDesc.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachDesc.initialLayout = ToVkImageLayout(initStatus);
		attachDesc.finalLayout   = ToVkImageLayout(dstStatus);

		//VkSubpassDependency dependency = {};
		//dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		//dependency.dstSubpass = 0;
		//dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		//dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		//dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		//dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		attachments.emplace_back(eastl::move(attachDesc));
		//dependencies.emplace_back(eastl::move(dependency));
		return *this;
	}

	VulkanRenderPass::Builder& VulkanRenderPass::Builder::BindDepthRenderTarget(VulkanRenderTarget* depth, EResourceBarrier initStatus, EResourceBarrier dstStatus)
	{
		if (bHaveDepth)
		{
			SG_LOG_WARN("Already bind a depth render target!");
			return *this;
		}

		VkAttachmentDescription attachDesc = {};
		attachDesc.format  = depth->format;
		attachDesc.samples = depth->sample;
		attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // We don't need depth after render pass has finished (DONT_CARE may result in better performance)
		attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachDesc.initialLayout = ToVkImageLayout(initStatus);
		attachDesc.finalLayout = ToVkImageLayout(dstStatus);

		// source dependency is when this subpass depend on. (i.e. this subpass will wait for source dependency finished then it will run)
		// destination dependency is when this subpass run on. (i.e. this subpass will run on this stage and make sure it will end in this stage)

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		bHaveDepth = true;
		attachments.emplace_back(eastl::move(attachDesc));
		dependencies.emplace_back(eastl::move(dependency));
		return *this;
	}

	VulkanRenderPass::Builder& VulkanRenderPass::Builder::CombineAsSubpass()
	{
		if (attachments.empty() || (attachments.size() == 1 && bHaveDepth))
		{
			SG_LOG_ERROR("At least bind one color render target to this subpass!");
			return *this;
		}
		UInt32 index = 0;

		VkSubpassDescription subpassDesc = {};
		for (Size i = 0; i < attachments.size(); ++i)
		{
			if (bHaveDepth && (attachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR))
			{
				VkAttachmentReference depthRef = {};
				depthRef.attachment = index++;
				depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthReference = depthRef;

				subpassDesc.pDepthStencilAttachment = &depthReference;
			}
			else
			{
				VkAttachmentReference attachRef = {};
				attachRef.attachment = index++;
				attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorReferences.emplace_back(attachRef);
			}
		}

		if (!bHaveDepth)
			subpassDesc.pDepthStencilAttachment = nullptr;

		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.colorAttachmentCount = bHaveDepth ? static_cast<UInt32>(attachments.size()) - 1 : static_cast<UInt32>(attachments.size());
		subpassDesc.pColorAttachments = colorReferences.data();

		subpasses.emplace_back(subpassDesc);
		return *this;
	}

	VulkanRenderPass* VulkanRenderPass::Builder::Build()
	{
		if (attachments.empty() || (attachments.size() == 1 && bHaveDepth))
		{
			SG_LOG_ERROR("At least bind one color render target to this subpass!");
			return nullptr;
		}
		if (subpasses.empty())
		{
			SG_LOG_ERROR("At least combine subpass once!");
			return nullptr;
		}

		return Memory::New<VulkanRenderPass>(device, attachments, dependencies, subpasses);
	}

	VulkanRenderPass::VulkanRenderPass(VulkanDevice& d, const vector<VkAttachmentDescription>& attachments, const vector<VkSubpassDependency>& dependencies, const vector<VkSubpassDescription>& subpasses)
		:device(d)
	{
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<UInt32>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = static_cast<UInt32>(subpasses.size());;
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = static_cast<UInt32>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK(vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &renderPass),
			SG_LOG_ERROR("Failed to create renderPass!"););
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		vkDestroyRenderPass(device.logicalDevice, renderPass, nullptr);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanFrameBuffer
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanFrameBuffer::VulkanFrameBuffer(VulkanDevice& d, const vector<VulkanRenderTarget*>& pRenderTargets, VulkanRenderPass* pRenderPass)
		:device(d)
	{
		vector<VkImageView> imageViews;
		for (auto* pRt : pRenderTargets)
			imageViews.emplace_back(pRt->imageView);

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		frameBufferCreateInfo.renderPass = pRenderPass->renderPass;

		frameBufferCreateInfo.attachmentCount = static_cast<UInt32>(pRenderTargets.size());
		frameBufferCreateInfo.pAttachments = imageViews.data();

		frameBufferCreateInfo.width  = pRenderTargets[0]->width;
		frameBufferCreateInfo.height = pRenderTargets[0]->height;
		frameBufferCreateInfo.layers = 1;

		this->width  = frameBufferCreateInfo.width;
		this->height = frameBufferCreateInfo.height;
		currRenderPass = pRenderPass->renderPass;

		VK_CHECK(vkCreateFramebuffer(device.logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffer),
			SG_LOG_ERROR("Failed to create vulkan frame buffer!"););
	}

	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		vkDestroyFramebuffer(device.logicalDevice, frameBuffer, nullptr);
	}

	VulkanFrameBuffer::Builder& VulkanFrameBuffer::Builder::AddRenderTarget(VulkanRenderTarget* pRenderTarget)
	{
		if (!renderTargets.empty())
		{
			if (renderTargets[0]->width != pRenderTarget->width || renderTargets[0]->height != pRenderTarget->height)
			{
				SG_LOG_WARN("Inconsistent render targets!");
				return *this;
			}
		}

		if (pRenderTarget->memory == VK_NULL_HANDLE) // it is a render target from the swapchain
		{
			if (bHaveSwapChainRT)
			{
				SG_LOG_WARN("Already bind a swapchain render target!");
				return *this;
			}
			bHaveSwapChainRT = true;
		}
		renderTargets.emplace_back(pRenderTarget);
		return *this;
	}

	VulkanFrameBuffer::Builder& VulkanFrameBuffer::Builder::BindRenderPass(VulkanRenderPass* pRenderPass)
	{
		if (this->pRenderPass)
		{
			SG_LOG_WARN("Already bind a render pass!");
			return *this;
		}
		this->pRenderPass = pRenderPass;
		return *this;
	}

	VulkanFrameBuffer* VulkanFrameBuffer::Builder::Build()
	{
		if (!bHaveSwapChainRT)
		{
			SG_LOG_ERROR("You have to bind the render target of the swapchain!");
			return nullptr;
		}

		return Memory::New<VulkanFrameBuffer>(device, renderTargets, pRenderPass);
	}

}