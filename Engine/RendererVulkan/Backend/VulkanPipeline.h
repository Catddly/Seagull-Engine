#pragma once

#include "Render/ResourceBarriers.h"

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"
#include <eastl/utility.h>

namespace SG
{

	class VulkanRenderPass
	{
	public:
		typedef eastl::pair<VkAttachmentDescription, VkSubpassDependency> attachment_t;

		VulkanRenderPass(VulkanDevice& d, const vector<VkAttachmentDescription>& attachments, const vector<VkSubpassDependency>& dependencies, const vector<VkSubpassDescription>& subpasses);
		~VulkanRenderPass();

		const VkRenderPass& NativeHandle() const { return renderPass; }

		class Builder
		{
		public:
			Builder(VulkanDevice& d) : device(d), bHaveDepth(false) {}
			~Builder() = default;

			Builder& BindColorRenderTarget(VulkanRenderTarget& color, EResourceBarrier initStatus, EResourceBarrier dstStatus);
			Builder& BindDepthRenderTarget(VulkanRenderTarget& depth, EResourceBarrier initStatus, EResourceBarrier dstStatus);
			Builder& CombineAsSubpass(); // TODO: support multiple subpass
			VulkanRenderPass* Build();
		private:
			VulkanDevice& device;
			vector<VkAttachmentDescription> colorAttachments;
			vector<VkSubpassDependency>     dependencies;
			attachment_t depthAttachment;
			bool         bHaveDepth;
			vector<VkSubpassDescription> subpasses;
		};
	private:
		VulkanDevice& device;
		VkRenderPass  renderPass;
	};

	class VulkanPipelineLayout
	{
	public:

	private:
	};

}