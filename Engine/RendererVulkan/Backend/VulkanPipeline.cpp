#include "Stdafx.h"
#include "VulkanPipeline.h"

#include "VulkanDescriptor.h"
#include "VulkanFrameBuffer.h"

#include "Render/Buffer.h"
#include "System/Logger.h"
#include "Memory/Memory.h"

namespace SG
{

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanPipelineLayout
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanPipelineLayout::Builder& VulkanPipelineLayout::Builder::AddDescriptorSetLayout(VulkanDescriptorSetLayout* layout)
	{
		if (layout)
			descriptorLayouts.emplace_back(layout->NativeHandle());
		return *this;
	}

	VulkanPipelineLayout::Builder& VulkanPipelineLayout::Builder::AddPushConstantRange(UInt32 size, UInt32 offset, EShaderStage stage)
	{
		if (size % 4 != 0) // must be the multiple of 4
		{
			SG_LOG_WARN("The size of the push constant range must be the multiple of 4!");
			return *this;
		}
		if (offset % 4 != 0) // must be the multiple of 4
		{
			SG_LOG_WARN("The offset of the push constant range must be the multiple of 4!");
			return *this;
		}
		if (size > device.physicalDeviceLimits.maxPushConstantsSize || offset >= device.physicalDeviceLimits.maxPushConstantsSize)
		{
			SG_LOG_WARN("Push constant size or offset exceed the limits!");
			return *this;
		}

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.size   = size;
		pushConstantRange.offset = offset;
		pushConstantRange.stageFlags = ToVkShaderStageFlags(stage);

		pushConstantRanges.emplace_back(pushConstantRange);
		return *this;
	}

	SG::VulkanPipelineLayout* VulkanPipelineLayout::Builder::Build()
	{
		return Memory::New<VulkanPipelineLayout>(device, descriptorLayouts, pushConstantRanges);
	}

	VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice& d, const vector<VkDescriptorSetLayout>& layouts, const vector<VkPushConstantRange>& pushConstant)
		:device(d)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<UInt32>(layouts.size());
		pipelineLayoutInfo.pSetLayouts    = layouts.data();
		// TDOO :support push constants
		pipelineLayoutInfo.pushConstantRangeCount = static_cast<UInt32>(pushConstant.size());;
		pipelineLayoutInfo.pPushConstantRanges    = pushConstant.data();

		VK_CHECK(vkCreatePipelineLayout(device.logicalDevice, &pipelineLayoutInfo, nullptr, &layout),
			SG_LOG_ERROR("Failed to create pipeline layout!"););
	}

	VulkanPipelineLayout::~VulkanPipelineLayout()
	{
		vkDestroyPipelineLayout(device.logicalDevice, layout, nullptr);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanPipeline
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanPipeline::VulkanPipeline(VulkanDevice& d, const PipelineStateCreateInfos& CI, VulkanPipelineLayout* pLayout, VulkanRenderPass* pRenderPass, Shader* shader)
		:device(d)
	{
		vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
		for (auto beg = shader->begin(); beg != shader->end(); ++beg)
		{
			VkPipelineShaderStageCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			switch (beg->first)
			{
			case EShaderStage::efVert:	createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
			case EShaderStage::efTesc:	createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
			case EShaderStage::efTese:	createInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
			case EShaderStage::efGeom:	createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
			case EShaderStage::efFrag:	createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			case EShaderStage::efComp:	createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
			default:                    SG_LOG_ERROR("Unknown type of shader stage!"); break;
			}

			VkShaderModule shaderModule = {};
			VkShaderModuleCreateInfo moduleCreateInfo = {};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = beg->second.binarySize;
			moduleCreateInfo.pCode = (UInt32*)beg->second.pBinary;
			vkCreateShaderModule(device.logicalDevice, &moduleCreateInfo, nullptr, &shaderModule); // should not be created in here

			Memory::Free(beg->second.pBinary);

			createInfo.module = shaderModule;
			createInfo.pName = "main";
			shaderStages.push_back(createInfo);
		}
		shader->clear();

		VkPipelineCacheCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK(vkCreatePipelineCache(device.logicalDevice, &createInfo, nullptr, &pipelineCache),
			SG_LOG_ERROR("Failed to create pipeline cache!"); pipelineCache = VK_NULL_HANDLE;);

		VkGraphicsPipelineCreateInfo graphicPipelineCreateInfo = {};

		graphicPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicPipelineCreateInfo.layout = pLayout->layout;
		graphicPipelineCreateInfo.renderPass = pRenderPass->renderPass;
		// assign the pipeline states to the pipeline creation info structure
		graphicPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		graphicPipelineCreateInfo.pStages = shaderStages.data();

		graphicPipelineCreateInfo.pVertexInputState   = &CI.vertexInputCI;
		graphicPipelineCreateInfo.pInputAssemblyState = &CI.inputAssemblyCI;
		graphicPipelineCreateInfo.pRasterizationState = &CI.rasterizeStateCI;
		graphicPipelineCreateInfo.pColorBlendState    = &CI.colorBlendCI;
		graphicPipelineCreateInfo.pMultisampleState   = &CI.multiSampleStateCI;
		graphicPipelineCreateInfo.pViewportState      = &CI.viewportStateCI;
		graphicPipelineCreateInfo.pDepthStencilState  = &CI.depthStencilCI;
		graphicPipelineCreateInfo.pDynamicState       = &CI.dynamicStateCI;

		//graphicPipelineCreateInfo.subpass = 0;

		VK_CHECK(vkCreateGraphicsPipelines(device.logicalDevice, pipelineCache, 1, &graphicPipelineCreateInfo, nullptr, &pipeline),
			SG_LOG_ERROR("Failed to create graphics pipeline!"););

		for (UInt32 i = 0; i < shaderStages.size(); ++i)
			vkDestroyShaderModule(device.logicalDevice, shaderStages[i].module, nullptr);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		vkDestroyPipelineCache(device.logicalDevice, pipelineCache, nullptr);
		vkDestroyPipeline(device.logicalDevice, pipeline, nullptr);
	}

	VulkanPipeline::Builder::Builder(VulkanDevice& d)
		: device(d)
	{
		// set the default status of pipeline
		SetInputAssembly();
		SetRasterizer(VK_CULL_MODE_NONE);
		SetColorBlend(true);
		SetDepthStencil(true);
		SetViewport();
		SetDynamicStates();
		SetMultiSample(ESampleCount::eSample_1);
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetVertexLayout(const VertexLayout& layout, bool perVertex)
	{
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = layout.GetTotalSize();
		vertexInputBinding.inputRate = perVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

		UInt32 location = 0;
		for (auto& e : layout)
		{
			VkVertexInputAttributeDescription vertexInputAttrib = {};
			vertexInputAttrib.binding = 0;
			vertexInputAttrib.location = location;
			vertexInputAttrib.format = ToVkShaderDataFormat(e.type);
			vertexInputAttrib.offset = e.offset;
			++location;
			createInfos.vertexInputAttributs.emplace_back(vertexInputAttrib);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputState = {};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = static_cast<UInt32>(createInfos.vertexInputAttributs.size());
		vertexInputState.pVertexAttributeDescriptions    = createInfos.vertexInputAttributs.data();

		createInfos.vertexInputCI = eastl::move(vertexInputState);
		return *this;
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetInputAssembly(VkPrimitiveTopology topology)
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = topology;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		inputAssemblyState.flags = 0;
		
		createInfos.inputAssemblyCI = eastl::move(inputAssemblyState);
		return *this;
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetRasterizer(VkCullModeFlags cullMode, VkPolygonMode polygonMode, bool depthClamp)
	{
		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = polygonMode;
		rasterizationState.cullMode = cullMode;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable = depthClamp ? VK_TRUE : VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		createInfos.rasterizeStateCI = eastl::move(rasterizationState);
		return *this;
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetColorBlend(bool enable)
	{
		createInfos.colorBlends.clear();
		VkPipelineColorBlendAttachmentState blendAttachmentState = {};
		blendAttachmentState.blendEnable    = enable ? VK_TRUE : VK_FALSE;
		if (enable)
		{
			blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}
		createInfos.colorBlends.emplace_back(blendAttachmentState);

		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = createInfos.colorBlends.data();

		createInfos.colorBlendCI = eastl::move(colorBlendState);
		return *this;
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetDepthStencil(bool enable)
	{
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		if (enable)
		{
			depthStencilState.depthTestEnable  = VK_TRUE;
			depthStencilState.depthWriteEnable = VK_TRUE;
			depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilState.depthBoundsTestEnable = VK_FALSE;
			depthStencilState.minDepthBounds = 0.0f;
			depthStencilState.maxDepthBounds = 1.0f;

			depthStencilState.stencilTestEnable = VK_FALSE;
			depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		}
		else
		{
			depthStencilState.depthTestEnable = VK_FALSE;
			depthStencilState.depthWriteEnable = VK_FALSE;
		}

		createInfos.depthStencilCI = eastl::move(depthStencilState);
		return *this;
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetViewport()
	{
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		createInfos.viewportStateCI = eastl::move(viewportState);
		return *this;
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetDynamicStates()
	{
		createInfos.dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
		createInfos.dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates    = createInfos.dynamicStates.data();
		dynamicState.dynamicStateCount = static_cast<UInt32>(createInfos.dynamicStates.size());

		createInfos.dynamicStateCI = eastl::move(dynamicState);
		return *this;
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetMultiSample(ESampleCount sample)
	{
		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = ToVkSampleCount(sample);
		multisampleState.pSampleMask = nullptr;

		createInfos.multiSampleStateCI = eastl::move(multisampleState);
		return *this;
	}

	VulkanPipeline* VulkanPipeline::Builder::Build()
	{
		return Memory::New<VulkanPipeline>(device, createInfos, pLayout, pRenderPass, pShader);
	}

}