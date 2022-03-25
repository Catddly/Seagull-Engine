#include "StdAfx.h"
#include "VulkanShader.h"

#include "VulkanDevice.h"

namespace SG
{

	VulkanShader::VulkanShader(VulkanDevice& device)
		:mDevice(device)
	{
	}

	void VulkanShader::CreatePipelineShader()
	{
		for (auto& beg = mShaderStages.begin(); beg != mShaderStages.end(); ++beg)
		{
			if (beg->second.binary.empty())
				continue;

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
			moduleCreateInfo.codeSize = beg->second.binary.size();
			moduleCreateInfo.pCode = reinterpret_cast<UInt32*>(beg->second.binary.data());
			vkCreateShaderModule(mDevice.logicalDevice, &moduleCreateInfo, nullptr, &shaderModule);
			mShaderModules.push_back(shaderModule);

			createInfo.module = shaderModule;
			createInfo.pName = GetEntryPoint().c_str();

			mShaderStagesCI.push_back(createInfo);
		}
		ReleaseBinary();
	}

	void VulkanShader::DestroyPipelineShader()
	{
		for (auto& shaderModule : mShaderModules)
			vkDestroyShaderModule(mDevice.logicalDevice, shaderModule, nullptr);
		// clear all the memory
		mShaderModules.set_capacity(0); 
		mShaderStagesCI.set_capacity(0);
	}

	RefPtr<VulkanShader> VulkanShader::Create(VulkanDevice& device)
	{
		return MakeRef<VulkanShader>(device);
	}

}