#include "StdAfx.h"
#include "VulkanPipelineSignature.h"

#include "Render/Shader/Shader.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	VulkanPipelineSignature::Builder& VulkanPipelineSignature::Builder::AddCombindSamplerImage(const char* samplerName, const char* textureName)
	{
		mCombineImages.emplace_back(samplerName, textureName);
		return *this;
	}

	RefPtr<VulkanPipelineSignature> VulkanPipelineSignature::Builder::Build()
	{
		return VulkanPipelineSignature::Create(mContext, mpShader, mCombineImages);
	}

	VulkanPipelineSignature::VulkanPipelineSignature(VulkanContext& context, RefPtr<VulkanShader> pShader, const vector<eastl::pair<const char*, const char*>>& combineImages)
		:mContext(context), mpShader(pShader)
	{
		// bind to the descriptor set
		VulkanDescriptorSetLayout::Builder uboLayoutBuilder(mContext.device);
		
		// for all the ubo in vertex stage
		auto& uboLayout = pShader->GetUniformBufferLayout();
		for (auto& uboData : uboLayout)
		{
			// create a buffer for this ubo layout
			BufferCreateDesc BufferCI = {};
			BufferCI.name = uboData.first.c_str();
			BufferCI.totalSizeInByte = uboData.second.layout.GetTotalSizeInByte();
			BufferCI.type = EBufferType::efUniform;
			VK_RESOURCE()->CreateBuffer(BufferCI);

			uboLayoutBuilder.AddBinding(EDescriptorType::eUniform_Buffer, uboData.second.stage, GetBinding(uboData.second.setbinding), 1);
		}

		auto& combineImageLayout = pShader->GetSampledImageLayout(EShaderStage::efFrag);
		for (auto& imageData : combineImageLayout)
		{
			uboLayoutBuilder.AddBinding(EDescriptorType::eCombine_Image_Sampler, EShaderStage::efFrag, GetBinding(imageData.second), 1);
		}

		mpUBODescriptorSetLayout = uboLayoutBuilder.Build();
		if (!mpUBODescriptorSetLayout)
		{
			SG_LOG_ERROR("Failed to create uniform buffer set layout with shader: (%s, %s)!", pShader->GetName(EShaderStage::efVert).c_str(), pShader->GetName(EShaderStage::efFrag).c_str());
			SG_ASSERT(false);
		}

		VulkanDescriptorDataBinder setDataBinder(*mContext.pDefaultDescriptorPool, *mpUBODescriptorSetLayout);
		// bind ubo buffer
		for (auto& uboData : uboLayout)
			setDataBinder.BindBuffer(GetBinding(uboData.second.setbinding), VK_RESOURCE()->GetBuffer(uboData.first));
		// bind combine image
		UInt32 imageIndex = 0;
		for (auto& imageData : combineImageLayout)
		{
			VulkanTexture* pTex = VK_RESOURCE()->GetTexture(combineImages[imageIndex].second);
			if (pTex)
				setDataBinder.BindImage(GetBinding(imageData.second), VK_RESOURCE()->GetSampler(combineImages[imageIndex].first), pTex);
			else
				setDataBinder.BindImage(GetBinding(imageData.second), VK_RESOURCE()->GetSampler(combineImages[imageIndex].first),
					VK_RESOURCE()->GetRenderTarget(combineImages[imageIndex].second));
			++imageIndex;
		}
		setDataBinder.Bind(mDescriptorSet);

		VulkanPipelineLayout::Builder pipelineLayoutBuilder(mContext.device);
		if (mpUBODescriptorSetLayout)
			pipelineLayoutBuilder.AddDescriptorSetLayout(mpUBODescriptorSetLayout.get());

		auto& pushConstantLayout = pShader->GetPushConstantLayout(EShaderStage::efVert);
		if (pushConstantLayout.GetTotalSizeInByte() != 0)
		{
			UInt32 offset = 0;
			pipelineLayoutBuilder.AddPushConstantRange(pushConstantLayout.GetTotalSizeInByte(), offset, EShaderStage::efVert);
			offset += pushConstantLayout.GetTotalSizeInByte();
		}

		mpPipelineLayout = pipelineLayoutBuilder.Build();
		if (!mpPipelineLayout)
		{
			SG_LOG_ERROR("Failed to create pipeline layout with shader: (%s, %s)!", pShader->GetName(EShaderStage::efVert).c_str(), pShader->GetName(EShaderStage::efFrag).c_str());
			SG_ASSERT(false);
		}
	}

	void VulkanPipelineSignature::UploadUniformBufferData(const char* uboName, const void* pData)
	{
		auto& uboLayout = mpShader->GetUniformBufferLayout();
		for (auto& uboData : uboLayout)
		{
			if (strcmp(uboData.first.c_str(), uboName) == 0)
			{
				VK_RESOURCE()->UpdataBufferData(uboName, pData);
				return;
			}
		}
		SG_LOG_WARN("Unable to find uniform buffer data for %s, did you spell it wrong?", uboName);
	}

	RefPtr<VulkanPipelineSignature> VulkanPipelineSignature::Create(VulkanContext& context, RefPtr<VulkanShader> pShader, const vector<eastl::pair<const char*, const char*>>& combineImages)
	{
		return MakeRef<VulkanPipelineSignature>(context, pShader, combineImages);
	}

}