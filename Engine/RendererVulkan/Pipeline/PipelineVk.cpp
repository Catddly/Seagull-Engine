#include "StdAfx.h"
#include "PipelineVk.h"

#include "RendererVulkan/Shader/ShaderVk.h"

#include "Common/System/ILog.h"
#include "Common/Platform/Window.h"
#include "Common/Render/RenderContext.h"
#include "Common/Render/SwapChain.h"

#include "Common/Stl/vector.h"

namespace SG
{

	//static void CreateGraphicPipeline(Shader* pShader)
	//{
	//	auto* shaderStages = pShader->GetShaderStages();
	//	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	//	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	//	pipelineInfo.stageCount = 2;
	//	pipelineInfo.pStages = shaderStages;
	//}

	PipelineVk::PipelineVk(Renderer* pRenderer, Shader* pShader, EPipelineType type)
		:mpRenderer(pRenderer), mpShader(pShader), mType(type)
	{
		ShaderStages* shaderStages = mpShader->GetShaderStages();

		vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;
		for (auto beg = shaderStages->begin(); beg != shaderStages->end(); beg++)
		{
			VkPipelineShaderStageCreateInfo shaderStageInfo = {};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			switch (beg->first)
			{
			case EShaderStages::eVert: shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
			case EShaderStages::eTesc: shaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
			case EShaderStages::eTese: shaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
			case EShaderStages::eGeom: shaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
			case EShaderStages::eFrag: shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			case EShaderStages::eComp: shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
			default: SG_LOG_ERROR("Invalid shader stage!"); SG_ASSERT(false); break;
			}
			shaderStageInfo.module = (VkShaderModule)beg->second.pShader;
			// set to main temporary
			shaderStageInfo.pName = "main";
			shaderCreateInfos.emplace_back(shaderStageInfo);
		}

		// no vertex data yet
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		SwapChain* pSwapChain = mpRenderer->GetSwapChain();
		UInt32 width = pSwapChain->GetExtent().width;
		UInt32 height = pSwapChain->GetExtent().height;
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { width, height };

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		// TODO: add user defined.
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// enable all dynamic states
		VkDynamicState dynamicStates[6];
		dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
		dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
		dynamicStates[2] = VK_DYNAMIC_STATE_DEPTH_BIAS;
		dynamicStates[3] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
		dynamicStates[4] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
		dynamicStates[5] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 6;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) 
		{
			SG_LOG_ERROR("Failed to create pipeline layout!");
			SG_ASSERT(false);
		}
		
		mpRenderpass = Memory::New<RenderPassVk>(mpRenderer);

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (UInt32)shaderCreateInfos.size();
		pipelineInfo.pStages = shaderCreateInfos.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState; // Optional

		pipelineInfo.layout = mPipelineLayout;

		pipelineInfo.renderPass = (VkRenderPass)mpRenderpass->GetNativeHandle();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), 
			VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create pipeline!");
			SG_ASSERT(false);
		}
	}

	PipelineVk::~PipelineVk()
	{
		vkDestroyPipelineLayout((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mPipelineLayout, nullptr);
		Memory::Delete(mpRenderpass);
		vkDestroyPipeline((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mPipeline, nullptr);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	/// RenderPass
	//////////////////////////////////////////////////////////////////////////////////////////////////

	RenderPassVk::RenderPassVk(Renderer* pRenderer)
		:mpRenderer(pRenderer)
	{
		VkAttachmentDescription colorAttachment = {};
		//colorAttachment.format = gImageFormatToVkMap[mpRenderer->GetSwapChain()->GetColorFormat()];
		// hardcode for temporary usage
		colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;

		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create renderpass!");
			SG_ASSERT(false);
		}
	}

	RenderPassVk::~RenderPassVk()
	{
		vkDestroyRenderPass((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mRenderPass, nullptr);
	}

	SG::Handle RenderPassVk::GetNativeHandle() const
	{
		return mRenderPass;
	}

}