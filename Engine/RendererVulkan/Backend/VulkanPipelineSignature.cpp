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

	VulkanPipelineSignature::Builder& VulkanPipelineSignature::Builder::OverrideDescriptorType(const string& name, EDescriptorType type)
	{
		mOverridesTypes[name] = type;
		return *this;
	}

	RefPtr<VulkanPipelineSignature> VulkanPipelineSignature::Builder::Build()
	{
		return VulkanPipelineSignature::Create(mContext, mpShader, mCombineImages, mOverridesTypes);
	}

	VulkanDescriptorSet& VulkanPipelineSignature::GetDescriptorSet(UInt32 set, const string& name)
	{
		SG_ASSERT(set < mDescriptorSetData.size());
		auto& setDescriptorsData = mDescriptorSetData[set];
		auto node = setDescriptorsData.setIndexMap.find(name);
		if (node == setDescriptorsData.setIndexMap.end())
			return setDescriptorsData.descriptorSets.back();
		else
			return setDescriptorsData.descriptorSets[node->second];
	}

	VulkanDescriptorSet& VulkanPipelineSignature::NewDescripotrSet(UInt32 set, const string& name)
	{
		SG_ASSERT(set < mDescriptorSetData.size());

		auto& setDescriptorsData = mDescriptorSetData[set];
		auto node = setDescriptorsData.setIndexMap.find(name);
		if (node == setDescriptorsData.setIndexMap.end())
		{
			setDescriptorsData.setIndexMap[name] = static_cast<UInt32>(setDescriptorsData.descriptorSets.size());
			return setDescriptorsData.descriptorSets.emplace_back();
		}
		else
			return setDescriptorsData.descriptorSets[node->second];
	}

	RefPtr<VulkanPipelineSignature> VulkanPipelineSignature::Create(VulkanContext& context, RefPtr<VulkanShader> pShader,
		const vector<eastl::pair<const char*, const char*>>& combineImages, const unordered_map<string, EDescriptorType>& overrides)
	{
		return MakeRef<VulkanPipelineSignature>(context, pShader, combineImages, overrides);
	}

	VulkanPipelineSignature::VulkanPipelineSignature(VulkanContext& context, RefPtr<VulkanShader> pShader, 
		const vector<eastl::pair<const char*, const char*>>& combineImages, const unordered_map<string, EDescriptorType>& overrides)
		:mContext(context), mpShader(pShader)
	{
		vector<UInt32> mSetImagesCount;

		for (auto setIndex : pShader->GetSetIndices())
		{
			bool bHaveNonDynamicBuffer = false;
			mDescriptorSetData.insert(setIndex);
			auto& setDescriptorsData = mDescriptorSetData[setIndex];

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
				bufferCI.memoryFlag = EGPUMemoryFlag::efPersistent_Map;
				if (!VK_RESOURCE()->HaveBuffer(bufferCI.name))
					VK_RESOURCE()->CreateBuffer(bufferCI);

				// check if user override this descriptor's type. since shader can not identify the difference of buffer and dynamic buffer.
				if (auto node = overrides.find(bufferCI.name); node != overrides.end())
				{
					descLayoutBuilder.AddBinding(node->second, uboData.second.stage, GetBinding(uboData.second.setbinding), 1);
					setDescriptorsData.setIndexMap[bufferCI.name] = static_cast<UInt32>(setDescriptorsData.descriptorSets.size());
					setDescriptorsData.descriptorSets.push_back();
				}
				else
				{
					descLayoutBuilder.AddBinding(EDescriptorType::eUniform_Buffer, uboData.second.stage, GetBinding(uboData.second.setbinding), 1);
					bHaveNonDynamicBuffer |= true;
				}
			}

			// for all the ssbo
			auto& ssboLayout = pShader->GetStorageBufferLayout();
			for (auto& ssboData : ssboLayout)
			{
				if (GetSet(ssboData.second.setbinding) != setIndex)
					continue;

				// create a buffer for this ssbo layout
				BufferCreateDesc bufferCI = {};
				bufferCI.name = ssboData.first.c_str();
				bufferCI.bufferSize = ssboData.second.layout.GetTotalSizeInByte();
				bufferCI.type = EBufferType::efStorage;
				bufferCI.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;
				bufferCI.memoryFlag = EGPUMemoryFlag::efPersistent_Map;
				if (!VK_RESOURCE()->HaveBuffer(bufferCI.name))
					VK_RESOURCE()->CreateBuffer(bufferCI);

				// check if user override this descriptor's type. since shader can not identify the difference of buffer and dynamic buffer.w
				if (auto node = overrides.find(bufferCI.name); node != overrides.end())
				{
					descLayoutBuilder.AddBinding(node->second, ssboData.second.stage, GetBinding(ssboData.second.setbinding), 1);
					setDescriptorsData.setIndexMap[bufferCI.name] = static_cast<UInt32>(setDescriptorsData.descriptorSets.size());
					setDescriptorsData.descriptorSets.push_back();
				}
				else
				{
					descLayoutBuilder.AddBinding(EDescriptorType::eStorage_Buffer, ssboData.second.stage, GetBinding(ssboData.second.setbinding), 1);
					bHaveNonDynamicBuffer |= true;
				}
			}

			UInt32 imageCount = 0;
			auto& combineImageLayout = pShader->GetSampledImageLayout(EShaderStage::efFrag);
			OrderSet<SetBindingKey> orderedSIInputLayout;
			for (auto& imageData : combineImageLayout)
				orderedSIInputLayout.emplace(imageData.second, imageData.second);
			for (auto& imageData : orderedSIInputLayout)
			{
				if (GetSet(imageData.second) != setIndex)
					continue;

				descLayoutBuilder.AddBinding(EDescriptorType::eCombine_Image_Sampler, EShaderStage::efFrag, GetBinding(imageData.second), 1);
				++imageCount;
			}

			mSetImagesCount.push_back(imageCount); // record current set's image count 

			mDescriptorSetData[setIndex].descriptorSetLayout = descLayoutBuilder.Build();
			if (!mDescriptorSetData[setIndex].descriptorSetLayout)
			{
				SG_LOG_ERROR("Failed to create descriptor set layout with shader: (%s, %s) (set: %d)!", pShader->GetName(EShaderStage::efVert).c_str(), pShader->GetName(EShaderStage::efFrag).c_str(), setIndex);
				SG_ASSERT(false);
			}

			if (bHaveNonDynamicBuffer || !combineImageLayout.Empty()) // add the non-uniform buffers into one descriptors
			{
				setDescriptorsData.setIndexMap["__non_dynamic"] = static_cast<UInt32>(setDescriptorsData.descriptorSets.size());
				setDescriptorsData.descriptorSets.push_back();
			}
		}

		UInt32 imageIndex = 0;
		for (auto setIndex : pShader->GetSetIndices()) // bind descriptors for each set
		{
			ShaderDataBinder shaderDataBinder(this, pShader, setIndex);

			Size setNumImages = mSetImagesCount[setIndex] + imageIndex;
			while (imageIndex < setNumImages)
			{
				shaderDataBinder.AddCombindSamplerImage(combineImages[imageIndex].first, combineImages[imageIndex].second);
				++imageIndex;
			}

			auto& setDescriptorsData = mDescriptorSetData[setIndex];
			shaderDataBinder.Bind(setDescriptorsData.descriptorSets[setDescriptorsData.setIndexMap["__non_dynamic"]]);
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

		if (imageIndex != combineImages.size())
		{
			SG_LOG_WARN("The number of textures pass in the shader do not match the number of the bindings!");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// SetDataBinder
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanPipelineSignature::SetDataBinder::SetDataBinder(RefPtr<VulkanPipelineSignature> pipelineSignature, UInt32 set)
		:mContext(pipelineSignature->mContext), mPipelineSignature(*pipelineSignature),
		mSet(set), mDataBinder(*mContext.pDefaultDescriptorPool, *pipelineSignature->mDescriptorSetData[set].descriptorSetLayout)
	{
	}

	VulkanPipelineSignature::SetDataBinder& VulkanPipelineSignature::SetDataBinder::AddCombindSamplerImage(UInt32 binding, const char* samplerName, const char* textureName)
	{
		VulkanTexture* pTex = VK_RESOURCE()->GetTexture(textureName);
		if (pTex)
			mDataBinder.BindImage(binding, VK_RESOURCE()->GetSampler(samplerName), pTex);
		else
			mDataBinder.BindImage(binding, VK_RESOURCE()->GetSampler(samplerName), VK_RESOURCE()->GetRenderTarget(textureName));
		return *this;
	}

	void VulkanPipelineSignature::SetDataBinder::Bind(VulkanDescriptorSet& set)
	{
		mDataBinder.Bind(set);
	}

	void VulkanPipelineSignature::SetDataBinder::Rebind(VulkanDescriptorSet& set)
	{
		//mContext.pDefaultDescriptorPool->FreeDescriptorSet(set);
		Bind(set);
	}

	VulkanDescriptorSet& VulkanPipelineSignature::SetDataBinder::BindNew(const string& name)
	{
		auto& newDescriptorSet = mPipelineSignature.NewDescripotrSet(mSet, name);
		Bind(newDescriptorSet);
		return newDescriptorSet;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ShaderDataBinder
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanPipelineSignature::ShaderDataBinder::ShaderDataBinder(VulkanPipelineSignature* pipelineSignature, RefPtr<VulkanShader> pShader, UInt32 set)
		:mContext(pipelineSignature->mContext), mPipelineSignature(*pipelineSignature), mpShader(pShader), mSet(set)
	{
	}

	VulkanPipelineSignature::ShaderDataBinder& VulkanPipelineSignature::ShaderDataBinder::AddCombindSamplerImage(const char* samplerName, const char* textureName)
	{
		mCombineImages.emplace_back(samplerName, textureName);
		return *this;
	}

	void VulkanPipelineSignature::ShaderDataBinder::Bind(VulkanDescriptorSet& set)
	{
		BindSet(set);
	}

	void VulkanPipelineSignature::ShaderDataBinder::ReBind(VulkanDescriptorSet& set)
	{
		//mContext.pDefaultDescriptorPool->FreeDescriptorSet(set);
		BindSet(set);
	}

	VulkanDescriptorSet& VulkanPipelineSignature::ShaderDataBinder::BindNew(const string& name)
	{
		auto& newDescriptorSet = mPipelineSignature.NewDescripotrSet(mSet, name);
		Bind(newDescriptorSet);
		return newDescriptorSet;
	}

	void VulkanPipelineSignature::ShaderDataBinder::BindSet(VulkanDescriptorSet& set)
	{
		auto setIndex = mSet;
		UInt32 imageIndex = 0;

		typedef eastl::pair<string, GPUBufferLayout> BufferLayoutType;
		auto& setDescriptorsData = mPipelineSignature.mDescriptorSetData[setIndex];
		vector<BufferLayoutType> nonDynamicBuffers;

		auto& uboLayout = mpShader->GetUniformBufferLayout();
		for (const BufferLayoutType& uboData : uboLayout)
		{
			if (GetSet(uboData.second.setbinding) != setIndex)
				continue;

			if (setDescriptorsData.setIndexMap.find(uboData.first) == setDescriptorsData.setIndexMap.end()) // add non-dynamic buffers data
				nonDynamicBuffers.emplace_back(uboData);
			else // bind dynamic descriptors
			{
				VulkanDescriptorDataBinder setDataBinder(*mContext.pDefaultDescriptorPool, *setDescriptorsData.descriptorSetLayout);
				setDataBinder.BindBuffer(GetBinding(uboData.second.setbinding), VK_RESOURCE()->GetBuffer(uboData.first));
				auto& descriptorSet = setDescriptorsData.descriptorSets[setDescriptorsData.setIndexMap[uboData.first]];
				setDataBinder.Bind(descriptorSet); // bind the corresponding descriptor set
				descriptorSet.belongingSet = setIndex;
			}
		}

		auto& ssboLayout = mpShader->GetStorageBufferLayout();
		for (const BufferLayoutType& ssboData : ssboLayout)
		{
			if (GetSet(ssboData.second.setbinding) != setIndex)
				continue;

			if (setDescriptorsData.setIndexMap.find(ssboData.first) == setDescriptorsData.setIndexMap.end())  // add non-dynamic buffers data
				nonDynamicBuffers.emplace_back(ssboData);
			else // bind dynamic descriptors
			{
				VulkanDescriptorDataBinder setDataBinder(*mContext.pDefaultDescriptorPool, *setDescriptorsData.descriptorSetLayout);
				setDataBinder.BindBuffer(GetBinding(ssboData.second.setbinding), VK_RESOURCE()->GetBuffer(ssboData.first));
				auto& descriptorSet = setDescriptorsData.descriptorSets[setDescriptorsData.setIndexMap[ssboData.first]];
				setDataBinder.Bind(descriptorSet); // bind the corresponding descriptor set
				descriptorSet.belongingSet = setIndex;
			}
		}

		auto& combineImageLayout = mpShader->GetSampledImageLayout(EShaderStage::efFrag);
		if (!nonDynamicBuffers.empty() || !combineImageLayout.Empty())
		{
			// bind non-dynamic descriptors
			VulkanDescriptorDataBinder setDataBinder(*mContext.pDefaultDescriptorPool, *setDescriptorsData.descriptorSetLayout);
			for (auto& bufferLayout : nonDynamicBuffers)
				setDataBinder.BindBuffer(GetBinding(bufferLayout.second.setbinding), VK_RESOURCE()->GetBuffer(bufferLayout.first));

			// bind combine image
			OrderSet<SetBindingKey> orderedSIInputLayout;
			for (auto& imageData : combineImageLayout)
				orderedSIInputLayout.emplace(imageData.second, imageData.second);
			for (auto& imageData : orderedSIInputLayout) // should use the ordered setbinding input data
			{
				if (GetSet(imageData.second) != setIndex)
					continue;

				// if there s no sampler or texture that had been created, do not bind it.
				// leave this slot in the shader as empty.
				VulkanSampler* pSampler = VK_RESOURCE()->GetSampler(mCombineImages[imageIndex].first);
				VulkanTexture* pTex = VK_RESOURCE()->GetTexture(mCombineImages[imageIndex].second);
				if (pTex && pSampler)
				{
					setDataBinder.BindImage(GetBinding(imageData.second), pSampler, pTex);
				}
				else
				{
					VulkanRenderTarget* pRenderTarget = VK_RESOURCE()->GetRenderTarget(mCombineImages[imageIndex].second);
					if (pRenderTarget)
						setDataBinder.BindImage(GetBinding(imageData.second), pSampler, pRenderTarget);
					else // bind the default texture (logo)
					{
						setDataBinder.BindImage(GetBinding(imageData.second), pSampler, VK_RESOURCE()->GetTexture("logo"));
					}
				}
				++imageIndex;
			}

			setDataBinder.Bind(set);
			set.belongingSet = setIndex;
		}
	}

}