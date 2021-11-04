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

		VulkanDevice& device;
		VkRenderPass  renderPass;
	};

	class VulkanDescriptorSetLayout;

	class VulkanPipelineLayout
	{
	public:
		VulkanPipelineLayout(VulkanDevice& d, const vector<VkDescriptorSetLayout>& layouts, const vector<VkPushConstantRange>& pushConstant);
		~VulkanPipelineLayout();

		class Builder
		{
		public:
			Builder(VulkanDevice& d) : device(d) {}
			~Builder() = default;

			Builder& BindDescriptorSetLayout(VulkanDescriptorSetLayout* pLayout);
			//Builder& BindPushConstantRange(const VulkanDescriptorSetLayout& pLayout);
			VulkanPipelineLayout* Build();
		private:
			VulkanDevice& device;
			vector<VkDescriptorSetLayout> descriptorLayouts;
			vector<VkPushConstantRange>   pushConstantRanges;
		};
	private:
		friend class VulkanPipeline;
		friend class VulkanCommandBuffer;

		VulkanDevice&    device;
		VkPipelineLayout layout;
	};

	class VulkanPipeline
	{
	public:
		struct PipelineStateCreateInfos
		{
			vector<VkVertexInputAttributeDescription> vertexInputAttributs;
			VkPipelineVertexInputStateCreateInfo   vertexInputCI;
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI;
			VkPipelineRasterizationStateCreateInfo rasterizeStateCI;
			vector<VkPipelineColorBlendAttachmentState> colorBlends;
			VkPipelineColorBlendStateCreateInfo    colorBlendCI;
			VkPipelineViewportStateCreateInfo      viewportStateCI;
			VkPipelineDepthStencilStateCreateInfo  depthStencilCI;
			VkPipelineMultisampleStateCreateInfo   multiSampleStateCI;
			vector<VkDynamicState>                 dynamicStates;
			VkPipelineDynamicStateCreateInfo       dynamicStateCI;
		};

		VulkanPipeline(VulkanDevice& d, const PipelineStateCreateInfos& CI, VulkanPipelineLayout* pLayout, VulkanRenderPass* pRenderPass, Shader* pShader);
		~VulkanPipeline();

		class Builder
		{
		public:
			Builder(VulkanDevice& d);
			~Builder() = default;
			
			// TODO: should use SPIRV reflection to automatically bind the vertex layout.
			// @brief Set vertex buffer layout.
			// @param [ layout ] The buffer layout.
			// @param [ perVertex ] True if vertex bind per vertex shader or false will bind per instance.
			Builder& SetVertexLayout(const BufferLayout& layout, bool perVertex = true);
			Builder& SetInputAssembly(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			Builder& SetRasterizer(VkCullModeFlags cullMode, VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, bool depthClamp = false);
			Builder& SetColorBlend(); // TODO: add blend mode.
			Builder& SetDepthStencil(bool enable);
			Builder& SetViewport();
			Builder& SetDynamicStates();
			Builder& SetMultiSample(ESampleCount sample);

			Builder& BindRenderPass(VulkanRenderPass* pRenderPass) { this->pRenderPass = pRenderPass; return *this; }
			Builder& BindLayout(VulkanPipelineLayout* pLayout) { this->pLayout = pLayout; return *this; }
			Builder& BindShader(Shader* pShader) { this->pShader = pShader; return *this; }

			VulkanPipeline* Build();
		private:
			VulkanDevice& device;
			PipelineStateCreateInfos createInfos;
			VulkanPipelineLayout*    pLayout;
			VulkanRenderPass*        pRenderPass;
			Shader*                  pShader;
		};
	private:
		friend class VulkanCommandBuffer;

		VulkanDevice&   device;
		VkPipelineCache pipelineCache;
		VkPipeline      pipeline;
	};

}