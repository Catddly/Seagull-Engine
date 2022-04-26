#pragma once

#include "VulkanDescriptor.h"

#include "Stl/SmartPtr.h"
#include "Stl/unordered_map.h"

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
		VulkanPipelineSignature(VulkanContext& context, RefPtr<VulkanShader>& pShader, const vector<eastl::pair<const char*, const char*>>& combineImages,
			const unordered_map<string, EDescriptorType>& overrides);
		~VulkanPipelineSignature() = default;

		//! One descriptor set layout can contain different descriptor set.
		struct SetDescriptorsData
		{
			RefPtr<VulkanDescriptorSetLayout> descriptorSetLayout;
			vector<VulkanDescriptorSet> descriptorSets;
			unordered_map<string, UInt32> setIndexMap; // name -> index of descriptorSets
		};

		SetDescriptorsData&  GetSetDescriptorsData(UInt32 set) { return mDescriptorSetData[set]; }
		VulkanDescriptorSet& GetDescriptorSet(UInt32 set, const string& name);
		
		class DataBinder
		{
		public:
			DataBinder(RefPtr<VulkanPipelineSignature> pipelineSignature, UInt32 set);

			DataBinder& AddCombindSamplerImage(UInt32 binding, const char* samplerName, const char* textureName);
			void Bind(VulkanDescriptorSet& set);
		private:
			VulkanContext& mContext;
			VulkanPipelineSignature& mPipelineSignature;
			UInt32 mSet;
			VulkanDescriptorDataBinder mDataBinder;
		};

		class Builder
		{
		public:
			Builder(VulkanContext& context, RefPtr<VulkanShader> pShader) : mContext(context), mpShader(pShader) {}
			~Builder() = default;

			//! You should add the combine image textures in the order written in the shader!
			Builder& AddCombindSamplerImage(const char* samplerName, const char* textureName);
			Builder& OverrideDescriptorType(const string& name, EDescriptorType type);
			RefPtr<VulkanPipelineSignature> Build();
		private:
			VulkanContext& mContext;
			RefPtr<VulkanShader> mpShader;
			vector<eastl::pair<const char*, const char*>> mCombineImages;
			unordered_map<string, EDescriptorType> mOverridesTypes;
		};

		static RefPtr<VulkanPipelineSignature> Create(VulkanContext& context, RefPtr<VulkanShader> pShader, 
			const vector<eastl::pair<const char*, const char*>>& combineImages, const unordered_map<string, EDescriptorType>& overrides);
	private:
		friend class VulkanCommandBuffer;
		friend class VulkanPipeline;
		VulkanContext& mContext;

		RefPtr<VulkanShader>&        mpShader;
		RefPtr<VulkanPipelineLayout> mpPipelineLayout;
		eastl::unordered_map<UInt32, SetDescriptorsData> mDescriptorSetData; // set -> <descriptorSetLayouts, descriptors>
	};

}