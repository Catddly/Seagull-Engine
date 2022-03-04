#pragma once

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include "VulkanPipelineSignature.h"

#include "volk.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"
#include <eastl/utility.h>

namespace SG
{

	class VulkanDescriptorSetLayout;
	class VulkanRenderPass;

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

			Builder& AddDescriptorSetLayout(VulkanDescriptorSetLayout* pLayout);
			Builder& AddPushConstantRange(UInt32 size, UInt32 offset, EShaderStage stage);
			RefPtr<VulkanPipelineLayout> Build();
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

	class VulkanShader;

	class VulkanPipeline
	{
	public:
		struct PipelineCreateInfos
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

		VulkanPipeline(VulkanDevice& d, const PipelineCreateInfos& CI, VulkanPipelineLayout* pLayout, VulkanRenderPass* pRenderPass, VulkanShader* pShader);
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
			Builder& SetInputAssembly(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			Builder& SetRasterizer(VkCullModeFlags cullMode, VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, bool depthClamp = false);
			Builder& SetColorBlend(bool enable); // TODO: add blend mode.
			Builder& SetDepthStencil(bool enable);
			Builder& SetViewport();
			Builder& SetDynamicStates();
			Builder& SetMultiSample(ESampleCount sample);

			Builder& BindRenderPass(VulkanRenderPass* pRenderPass) { this->pRenderPass = pRenderPass; return *this; }
			Builder& BindSignature(VulkanPipelineSignature* pSignature) { this->pLayout = pSignature->mpPipelineLayout.get(); return *this; }
			Builder& BindShader(VulkanShader* pShader);

			VulkanPipeline* Build();
		private:
			Builder& SetVertexLayout(const ShaderAttributesLayout& layout, bool perVertex = true);
		private:
			VulkanDevice&         device;
			PipelineCreateInfos   createInfos;
			VulkanPipelineLayout* pLayout;
			VulkanRenderPass*     pRenderPass;
			VulkanShader*         pShader;
		};
	private:
		friend class VulkanCommandBuffer;

		VulkanDevice&   device;
		VkPipelineCache pipelineCache;
		VkPipeline      pipeline;
	};

}