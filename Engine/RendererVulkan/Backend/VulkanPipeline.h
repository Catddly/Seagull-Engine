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
		struct GraphicPipelineCreateInfo
		{
			vector<VkVertexInputBindingDescription> vertexInputBindingDesc;
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

		VulkanPipeline(VulkanDevice& d, const GraphicPipelineCreateInfo& CI, VulkanPipelineLayout* pLayout, VulkanRenderPass* pRenderPass, VulkanShader* pShader);
		VulkanPipeline(VulkanDevice& d, VulkanPipelineLayout* pLayout, VulkanShader* pShader);
		~VulkanPipeline();

		class Builder
		{
		public:
			Builder(VulkanDevice& d, EPipelineType type = EPipelineType::eGraphic);
			~Builder() = default;
			
			Builder& SetInputVertexRange(Size size, EVertexInputRate inputRate);
			// TODO: should use SPIRV reflection to automatically bind the vertex layout.
			// @brief Set vertex buffer layout.
			// @param [ layout ] The buffer layout.
			// @param [ perVertex ] True if vertex bind per vertex shader or false will bind per instance.
			Builder& SetInputAssembly(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			Builder& SetRasterizer(ECullMode cullMode, EPolygonMode polygonMode = EPolygonMode::eFill, bool depthClamp = false);
			Builder& SetColorBlend(bool enable); // TODO: add blend mode.
			Builder& SetDepthStencil(bool enable);
			Builder& SetViewport();
			Builder& SetDynamicStates(UInt32 addState = VK_DYNAMIC_STATE_MAX_ENUM);
			Builder& SetMultiSample(ESampleCount sample);

			Builder& BindRenderPass(VulkanRenderPass* pRenderPass) { this->pRenderPass = pRenderPass; return *this; }
			Builder& BindSignature(VulkanPipelineSignature* pSignature) { this->pLayout = pSignature->mpPipelineLayout.get(); return *this; }
			Builder& BindShader(VulkanShader* pShader);

			VulkanPipeline* Build();
		private:
			Builder& SetVertexLayout(const ShaderAttributesLayout& layout);
		private:
			VulkanDevice&         device;
			GraphicPipelineCreateInfo   createInfos;
			VulkanPipelineLayout* pLayout;
			VulkanRenderPass*     pRenderPass;
			VulkanShader*         pShader;
			EPipelineType         pipelineType;
		};
	private:
		friend class VulkanCommandBuffer;

		VulkanDevice&   device;
		VkPipelineCache pipelineCache;
		VkPipeline      pipeline;
	};

}