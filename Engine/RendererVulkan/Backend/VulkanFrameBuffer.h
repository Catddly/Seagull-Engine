#pragma once

#include "VulkanDevice.h"

#include "Render/ResourceBarriers.h"

namespace SG
{

	class VulkanRenderTarget;

	class VulkanRenderPass
	{
	public:
		VulkanRenderPass(VulkanDevice& d, const vector<VkAttachmentDescription>& attachments, const vector<VkSubpassDependency>& dependencies, const vector<VkSubpassDescription>& subpasses);
		~VulkanRenderPass();

		const VkRenderPass& NativeHandle() const { return renderPass; }

		// The data in the builder will not be cached!
		class Builder
		{
		public:
			Builder(VulkanDevice& d) : device(d), bHaveDepth(false), depthReference({}) {}
			~Builder() = default;

			Builder& BindColorRenderTarget(VulkanRenderTarget& color, EResourceBarrier initStatus, EResourceBarrier dstStatus);
			Builder& BindDepthRenderTarget(VulkanRenderTarget& depth, EResourceBarrier initStatus, EResourceBarrier dstStatus);
			/**
			 * @brief Combine the render targets you had binded to a subpass.
			 * The firstly binded render target will be attachment 0, the second render target will be 1 and so on.
			 */
			Builder& CombineAsSubpass(); // TODO: support multiple subpass
			VulkanRenderPass* Build();
		private:
			VulkanDevice& device;
			vector<VkAttachmentDescription> attachments;
			vector<VkSubpassDependency>     dependencies;
			vector<VkAttachmentReference>   colorReferences;
			bool                            bHaveDepth;
			VkAttachmentReference           depthReference;
			vector<VkSubpassDescription>    subpasses;
		};
	private:
		friend class VulkanPipeline;
		friend class VulkanFrameBuffer;

		VulkanDevice& device;
		VkRenderPass  renderPass;
	};

	class VulkanFrameBuffer
	{
	public:
		VulkanFrameBuffer(VulkanDevice& d, const vector<VulkanRenderTarget*>& pRenderTargets, VulkanRenderPass* pRenderPass);
		~VulkanFrameBuffer();

		class Builder
		{
		public:
			Builder(VulkanDevice& d) : device(d), bHaveSwapChainRT(false), pRenderPass(nullptr) {}
			~Builder() = default;

			Builder& AddRenderTarget(VulkanRenderTarget* pRenderTarget);
			Builder& BindRenderPass(VulkanRenderPass* pRenderPass);
			VulkanFrameBuffer* Build();
		private:
			VulkanDevice& device;
			vector<VulkanRenderTarget*> renderTargets;
			VulkanRenderPass*           pRenderPass;
			bool bHaveSwapChainRT; // if user had bind the render target of swapchain
		};
	private:
		friend class VulkanCommandBuffer;

		VulkanDevice& device;
		VkFramebuffer frameBuffer;
		VkRenderPass  currRenderPass;
		UInt32 width;
		UInt32 height;
	};

}