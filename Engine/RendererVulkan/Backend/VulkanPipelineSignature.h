#pragma once

#include "VulkanDescriptor.h"

#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanContext;
	class VulkanShader;
	class VulkanDescriptorSetLayout;
	class VulkanPipelineLayout;

	//! Auto DescriptorSetLayout and PipelineLayout creator.
	//! Whenever you want to use this, please make sure the resource is created and loaded into GPU successfully.
	//! This class is only for creating resource purpose, it has no responsibility to destroy them.
	class VulkanPipelineSignature
	{
	public:
		VulkanPipelineSignature(VulkanContext& context, RefPtr<VulkanShader>& pShader, const vector<eastl::pair<const char*, const char*>>& combineImages);
		~VulkanPipelineSignature() = default;

		class Builder
		{
		public:
			Builder(VulkanContext& context, RefPtr<VulkanShader> pShader) : mContext(context), mpShader(pShader) {}
			~Builder() = default;

			//! You should add the combine image textures in the order written in the shader!
			Builder& AddCombindSamplerImage(const char* samplerName, const char* textureName);
			RefPtr<VulkanPipelineSignature> Build();
		private:
			VulkanContext& mContext;
			RefPtr<VulkanShader> mpShader;
			vector<eastl::pair<const char*, const char*>> mCombineImages;
		};

		static RefPtr<VulkanPipelineSignature> Create(VulkanContext& context, RefPtr<VulkanShader> pShader, const vector<eastl::pair<const char*, const char*>>& combineImages);
	private:
		friend class VulkanCommandBuffer;
		friend class VulkanPipeline;
		VulkanContext& mContext;

		struct SetLayoutAndHandle
		{
			RefPtr<VulkanDescriptorSetLayout> descriptorSetLayout;
			VulkanDescriptorSet descriptorSet;
		};

		RefPtr<VulkanShader>&        mpShader;
		RefPtr<VulkanPipelineLayout> mpPipelineLayout;
		eastl::unordered_map<UInt32, SetLayoutAndHandle> mDescriptorSetData;

	};

}