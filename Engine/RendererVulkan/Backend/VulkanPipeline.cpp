#include "Stdafx.h"
#include "VulkanPipeline.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

namespace SG
{

	/////////////////////////////////////////////////////////////////////////
	/// VulkanRenderPass
	/////////////////////////////////////////////////////////////////////////

	VulkanRenderPass::Builder& VulkanRenderPass::Builder::BindColorRenderTarget(VulkanRenderTarget& color, EResourceBarrier initStatus, EResourceBarrier dstStatus)
	{
		VkAttachmentDescription attachDesc = {};
		attachDesc.format  = color.format;
		attachDesc.samples = color.sample;
		// TODO: add load store mask
		attachDesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachDesc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachDesc.initialLayout  = ToVkImageLayout(initStatus);
		attachDesc.finalLayout    = ToVkImageLayout(dstStatus);

		VkSubpassDependency subpass = {};
		subpass.srcSubpass = VK_SUBPASS_EXTERNAL;                             // Producer of the dependency
		subpass.dstSubpass = 0;                                               // Consumer is our single subpass that will wait for the execution dependency
		subpass.srcStageMask = ToVkPipelineStageFlags(ToVkAccessFlags(EResourceBarrier::efRenderTarget), EQueueType::eGraphic); // Match our pWaitDstStageMask when we vkQueueSubmit
		subpass.dstStageMask = ToVkPipelineStageFlags(ToVkAccessFlags(EResourceBarrier::efRenderTarget), EQueueType::eGraphic); // is a loadOp stage for color attachments
		//subpass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Match our pWaitDstStageMask when we vkQueueSubmit
		//subpass.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a loadOp stage for color attachments
		subpass.srcAccessMask = 0;                                            // semaphore wait already does memory dependency for us
		subpass.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a loadOp CLEAR access mask for color attachments
		subpass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		colorAttachments.emplace_back(eastl::move(attachDesc));
		dependencies.emplace_back(eastl::move(subpass));
		return *this;
	}

	VulkanRenderPass::Builder& VulkanRenderPass::Builder::BindDepthRenderTarget(VulkanRenderTarget& depth, EResourceBarrier initStatus, EResourceBarrier dstStatus)
	{
		if (bHaveDepth)
		{
			SG_LOG_WARN("Already bind a depth render target!");
			return *this;
		}

		VkAttachmentDescription attachDesc = {};
		attachDesc.format  = depth.format;
		attachDesc.samples = depth.sample;
		attachDesc.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // We don't need depth after render pass has finished (DONT_CARE may result in better performance)
		attachDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachDesc.initialLayout = ToVkImageLayout(initStatus);
		attachDesc.finalLayout   = ToVkImageLayout(dstStatus);

		VkSubpassDependency subpass = {};
		subpass.srcSubpass = 0;                                               // Producer of the dependency is our single subpass
		subpass.dstSubpass = VK_SUBPASS_EXTERNAL;                             // Consumer are all commands outside of the renderpass
		subpass.srcStageMask  = ToVkPipelineStageFlags(ToVkAccessFlags(EResourceBarrier::efRenderTarget), EQueueType::eGraphic); // is a storeOp stage for color attachments
		subpass.dstStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;          // Do not block any subsequent work
		subpass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a storeOp `STORE` access mask for color attachments
		subpass.dstAccessMask = 0;
		subpass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		bHaveDepth = true;
		depthAttachment = { eastl::move(attachDesc), eastl::move(subpass) };
		return *this;
	}

	VulkanRenderPass::Builder& VulkanRenderPass::Builder::CombineAsSubpass()
	{
		if (colorAttachments.empty())
		{
			SG_LOG_ERROR("At least bind one render target to this subpass!");
			return *this;
		}
		UInt32 index = 0;

		VkSubpassDescription  subpassDesc = {};

		VkAttachmentReference depthReference = {};
		vector<VkAttachmentReference> colorReferences;
		for (Size i = 0; i < colorAttachments.size(); ++i)
		{
			VkAttachmentReference attachRef = {};
			attachRef.attachment = index++;
			attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorReferences.emplace_back(attachRef);
		}

		if (bHaveDepth)
		{
			depthReference.attachment = index++;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			subpassDesc.pDepthStencilAttachment = &depthReference;
		}

		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.colorAttachmentCount = static_cast<UInt32>(colorAttachments.size());
		subpassDesc.pColorAttachments = colorReferences.data();
		subpassDesc.inputAttachmentCount = 0;       // Input attachments can be used to sample from contents of a previous subpass
		subpassDesc.pInputAttachments = nullptr;    // (Input attachments not used by this example)
		subpassDesc.preserveAttachmentCount = 0;    // Preserved attachments can be used to loop (and preserve) attachments through subpasses
		subpassDesc.pPreserveAttachments = nullptr; // (Preserve attachments not used by this example)
		subpassDesc.pResolveAttachments = nullptr;  // Resolve attachments are resolved at the end of a sub pass and can be used for e.g. multi sampling

		subpasses.emplace_back(subpassDesc);
		return *this;
	}

	VulkanRenderPass* VulkanRenderPass::Builder::Build()
	{
		if (colorAttachments.empty())
		{
			SG_LOG_ERROR("At least bind one render target to this subpass!");
			return nullptr;
		}
		if (subpasses.empty())
		{
			SG_LOG_ERROR("At least combine subpass once!");
			return nullptr;
		}
	
		// combine depth into attachments
		if (bHaveDepth)
		{
			colorAttachments.emplace_back(depthAttachment.first);
			dependencies.emplace_back(depthAttachment.second);
		}
		return Memory::New<VulkanRenderPass>(device, colorAttachments, dependencies, subpasses);
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

}