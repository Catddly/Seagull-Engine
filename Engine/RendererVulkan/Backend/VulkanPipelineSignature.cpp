#include "StdAfx.h"
#include "VulkanPipelineSignature.h"

#include "Render/Shader/Shader.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanTexture.h"
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

	RefPtr<VulkanPipelineSignature> VulkanPipelineSignature::Create(VulkanContext& context, RefPtr<VulkanShader> pShader, 
		const vector<eastl::pair<const char*, const char*>>& combineImages)
	{
		return MakeRef<VulkanPipelineSignature>(context, pShader, combineImages);
	}

	VulkanPipelineSignature::VulkanPipelineSignature(VulkanContext& context, RefPtr<VulkanShader>& pShader, const vector<eastl::pair<const char*, const char*>>& combineImages)
		:mContext(context), mpShader(pShader)
	{
		for (auto setIndex : pShader->GetSetIndices())
		{
			mDescriptorSetData.insert(setIndex);

			// bind to the descriptor set
			VulkanDescriptorSetLayout::Builder descLayoutBuilder(mContext.device);

			// for all the ubo in vertex stage
			auto& uboLayout = pShader->GetUniformBufferLayout();
			for (auto& uboData : uboLayout)
			{
				if (GetSet(uboData.second.setbinding) != setIndex)
					continue;

				// create a buffer for this ubo layout
				BufferCreateDesc bufferCI = {};
				bufferCI.name = uboData.first.c_str();
				bufferCI.bufferSize = uboData.second.layout.GetTotalSizeInByte();
				bufferCI.type = EBufferType::efUniform;
				bufferCI.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;
				if (!VK_RESOURCE()->GetBuffer(bufferCI.name))
					VK_RESOURCE()->CreateBuffer(bufferCI);

				descLayoutBuilder.AddBinding(EDescriptorType::eUniform_Buffer, uboData.second.stage, GetBinding(uboData.second.setbinding), 1);
			}

			// for all the ssbo
			auto& ssboLayout = pShader->GetStorageBufferLayout();
			for (auto& ssboData : ssboLayout)
			{
				if (GetSet(ssboData.second.setbinding) != setIndex)
					continue;

				// create a buffer for this ubo layout
				BufferCreateDesc bufferCI = {};
				bufferCI.name = ssboData.first.c_str();
				bufferCI.bufferSize = ssboData.second.layout.GetTotalSizeInByte();
				bufferCI.type = EBufferType::efStorage;
				bufferCI.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;

				if (!VK_RESOURCE()->GetBuffer(bufferCI.name))
					VK_RESOURCE()->CreateBuffer(bufferCI);

				descLayoutBuilder.AddBinding(EDescriptorType::eStorage_Buffer, ssboData.second.stage, GetBinding(ssboData.second.setbinding), 1);
			}

			auto& combineImageLayout = pShader->GetSampledImageLayout(EShaderStage::efFrag);
			OrderSet<SetBindingKey> orderedSIInputLayout;
			for (auto& imageData : combineImageLayout)
				orderedSIInputLayout.emplace(imageData.second, imageData.second);
			for (auto& imageData : orderedSIInputLayout)
			{
				if (GetSet(imageData.second) != setIndex)
					continue;
				descLayoutBuilder.AddBinding(EDescriptorType::eCombine_Image_Sampler, EShaderStage::efFrag, GetBinding(imageData.second), 1);
			}

			mDescriptorSetData[setIndex].descriptorSetLayout = descLayoutBuilder.Build();
			if (!mDescriptorSetData[setIndex].descriptorSetLayout)
			{
				SG_LOG_ERROR("Failed to create descriptor set layout with shader: (%s, %s) (set: %d)!", pShader->GetName(EShaderStage::efVert).c_str(), pShader->GetName(EShaderStage::efFrag).c_str(), setIndex);
				SG_ASSERT(false);
			}
		}

		UInt32 numImages = 0;
		for (auto setIndex : pShader->GetSetIndices())
		{
			auto pSetLayout = mDescriptorSetData[setIndex].descriptorSetLayout;
			VulkanDescriptorDataBinder setDataBinder(*mContext.pDefaultDescriptorPool, *pSetLayout);
			// bind ubo buffer
			auto& uboLayout = pShader->GetUniformBufferLayout();
			for (auto& uboData : uboLayout)
			{
				if (GetSet(uboData.second.setbinding) != setIndex)
					continue;
				setDataBinder.BindBuffer(GetBinding(uboData.second.setbinding), VK_RESOURCE()->GetBuffer(uboData.first));
			}
			// bind storage buffer
			auto& ssboLayout = pShader->GetStorageBufferLayout();
			for (auto& ssboData : ssboLayout)
			{
				if (GetSet(ssboData.second.setbinding) != setIndex)
					continue;
				string bufferName = ssboData.first;
				setDataBinder.BindBuffer(GetBinding(ssboData.second.setbinding), VK_RESOURCE()->GetBuffer(bufferName));
			}
			// bind combine image
			auto& combineImageLayout = pShader->GetSampledImageLayout(EShaderStage::efFrag);
			OrderSet<SetBindingKey> orderedSIInputLayout;
			for (auto& imageData : combineImageLayout)
				orderedSIInputLayout.emplace(imageData.second, imageData.second);
			UInt32 imageIndex = 0;
			for (auto& imageData : orderedSIInputLayout) // should use the ordered setbinding input data
			{
				if (GetSet(imageData.second) != setIndex)
					continue;
				VulkanTexture* pTex = VK_RESOURCE()->GetTexture(combineImages[imageIndex].second);
				if (pTex)
					setDataBinder.BindImage(GetBinding(imageData.second), VK_RESOURCE()->GetSampler(combineImages[imageIndex].first), pTex);
				else
					setDataBinder.BindImage(GetBinding(imageData.second), VK_RESOURCE()->GetSampler(combineImages[imageIndex].first),
						VK_RESOURCE()->GetRenderTarget(combineImages[imageIndex].second));
				++imageIndex;
			}

			setDataBinder.Bind(mDescriptorSetData[setIndex].descriptorSet);
			mDescriptorSetData[setIndex].descriptorSet.belongingSet = setIndex;
			numImages += imageIndex;
		}

		VulkanPipelineLayout::Builder pipelineLayoutBuilder(mContext.device);
		for (auto setIndex : pShader->GetSetIndices())
			pipelineLayoutBuilder.AddDescriptorSetLayout(mDescriptorSetData[setIndex].descriptorSetLayout.get());

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

		if (numImages != combineImages.size())
		{
			SG_LOG_WARN("The number of textures pass in the shader do not match the number of the bindings!");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// DataBinder
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanPipelineSignature::DataBinder::DataBinder(RefPtr<VulkanPipelineSignature> pipelineSignature, UInt32 set)
		:mContext(pipelineSignature->mContext), mPipelineSignature(*pipelineSignature),
		mSet(set), mDataBinder(*mContext.pDefaultDescriptorPool, *pipelineSignature->mDescriptorSetData[set].descriptorSetLayout)
	{
	}

	VulkanPipelineSignature::DataBinder& VulkanPipelineSignature::DataBinder::AddCombindSamplerImage(UInt32 binding, const char* samplerName, const char* textureName)
	{
		VulkanTexture* pTex = VK_RESOURCE()->GetTexture(textureName);
		if (pTex)
			mDataBinder.BindImage(binding, VK_RESOURCE()->GetSampler(samplerName), pTex);
		else
			mDataBinder.BindImage(binding, VK_RESOURCE()->GetSampler(samplerName), VK_RESOURCE()->GetRenderTarget(textureName));
		return *this;
	}

	void VulkanPipelineSignature::DataBinder::Bind(VulkanDescriptorSet& set)
	{
		mDataBinder.Bind(set);
	}

}