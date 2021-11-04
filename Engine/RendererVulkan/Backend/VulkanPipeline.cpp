#include "Stdafx.h"
#include "VulkanPipeline.h"

#include "VulkanDescriptor.h"

#include "Render/Buffer.h"
#include "System/Logger.h"
#include "Memory/Memory.h"

namespace SG
{

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanRenderPass
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

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

		attachments.emplace_back(eastl::move(attachDesc));
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
		attachments.emplace_back(eastl::move(attachDesc));
		dependencies.emplace_back(eastl::move(subpass));
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

		VkSubpassDescription  subpassDesc = {};
		for (Size i = 0; i < attachments.size(); ++i)
		{
			if (bHaveDepth && (attachments[i].format == VK_FORMAT_D24_UNORM_S8_UINT || attachments[i].format == VK_FORMAT_D32_SFLOAT_S8_UINT))
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
	/// VulkanPipelineLayout
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanPipelineLayout::Builder& VulkanPipelineLayout::Builder::BindDescriptorSetLayout(VulkanDescriptorSetLayout* layout)
	{
		if (layout)
			descriptorLayouts.emplace_back(layout->NativeHandle());
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
			case EShaderStages::efVert:	createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
			case EShaderStages::efTesc:	createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
			case EShaderStages::efTese:	createInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
			case EShaderStages::efGeom:	createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
			case EShaderStages::efFrag:	createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			case EShaderStages::efComp:	createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
			default:                    SG_LOG_ERROR("Unknown type of shader stage!"); break;
			}

			VkShaderModule shaderModule;
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
		SetColorBlend();
		SetDepthStencil(true);
		SetViewport();
		SetDynamicStates();
		SetMultiSample(ESampleCount::eSample_1);
	}

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetVertexLayout(const BufferLayout& layout, bool perVertex)
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

	VulkanPipeline::Builder& VulkanPipeline::Builder::SetColorBlend()
	{
		VkPipelineColorBlendAttachmentState blendAttachmentState = {};
		blendAttachmentState.colorWriteMask = 0xf;
		blendAttachmentState.blendEnable    = VK_FALSE;
		createInfos.colorBlends.emplace_back(blendAttachmentState);

		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = static_cast<UInt32>(createInfos.colorBlends.size());
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
			depthStencilState.stencilTestEnable = VK_FALSE;

			depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
			depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
			depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

			depthStencilState.front = depthStencilState.back;
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