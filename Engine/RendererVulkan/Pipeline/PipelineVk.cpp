#include "StdAfx.h"
#include "PipelineVk.h"

#include "RendererVulkan/Shader/ShaderVk.h"

#include "Common/System/ILog.h"

#include "Common/Stl/vector.h"

namespace SG
{

	PipelineVk::PipelineVk(Renderer* pRenderer, Shader* pShader, EPipelineType type)
		:mpRenderer(pRenderer), mpShader(pShader), mType(type)
	{
		ShaderStages* shaderStages = mpShader->GetShaderStages();

		vector<VkPipelineShaderStageCreateInfo> createInfos;
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
			createInfos.emplace_back(shaderStageInfo);
		}
	}

	PipelineVk::~PipelineVk()
	{

	}

}