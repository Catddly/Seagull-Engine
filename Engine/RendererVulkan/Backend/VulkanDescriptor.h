#pragma once

#include "Base/BasicTypes.h"
#include "Render/Descriptor.h"

#include "VulkanDevice.h"

#include "volk.h"

#include "Stl/SmartPtr.h"
#include "Stl/vector.h"
#include "eastl/array.h"
#include <eastl/unordered_map.h>

namespace SG
{

	class VulkanDescriptorPool
	{
	public:
		VulkanDescriptorPool(VulkanDevice& d, const vector<VkDescriptorPoolSize>& poolSizes, UInt32 maxSets);
		~VulkanDescriptorPool();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanDescriptorPool);

		class Builder
		{
		public:
			Builder& AddPoolElement(EDescriptorType type, UInt32 count);
			Builder& SetMaxSets(UInt32 max);
			VulkanDescriptorPool* Build(VulkanDevice& d);
		private:
			vector<VkDescriptorPoolSize> poolSizes;
			UInt32 maxSets;
		};
	private:
		bool AllocateDescriptorSet(const VkDescriptorSetLayout& layout, VkDescriptorSet& set);
		void FreeDescriptorSet(VkDescriptorSet& set);
		void Reset();
	private:
		friend class VulkanDescriptorDataBinder;

		VulkanDevice&    device;
		VkDescriptorPool pool;
	};

	class VulkanDescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout(VulkanDevice& d, const eastl::unordered_map<UInt32, VkDescriptorSetLayoutBinding>& layouts);
		~VulkanDescriptorSetLayout();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanDescriptorSetLayout);

		class Builder
		{
		public:
			Builder(VulkanDevice& d) : device(d) {}

			Builder& AddBinding(EDescriptorType type, EShaderStage stage, UInt32 binding, UInt32 count);
			RefPtr<VulkanDescriptorSetLayout> Build();
		private:
			VulkanDevice& device;
			eastl::unordered_map<UInt32, VkDescriptorSetLayoutBinding> bindingsMap;
		};
	private:
		friend class VulkanPipelineLayout;
		friend class VulkanDescriptorDataBinder;

		VulkanDevice&         device;
		VkDescriptorSetLayout descriptorSetLayout;
		eastl::unordered_map<UInt32, VkDescriptorSetLayoutBinding> bindingsMap;
	};

	//! Handle to the resources which bound to the VulkanDescriptorSetLayout.
	class VulkanDescriptorSet
	{
	public:
	private:
		friend class VulkanDescriptorDataBinder;
		friend class VulkanCommandBuffer;
		VkDescriptorSet set;
	};

	class VulkanSampler;
	class VulkanTexture;

	class VulkanDescriptorDataBinder
	{
	public:
		VulkanDescriptorDataBinder(VulkanDescriptorPool& pool, VulkanDescriptorSetLayout& layout);
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanDescriptorDataBinder);

		VulkanDescriptorDataBinder& BindBuffer(UInt32 binding, const VulkanBuffer* info);
		VulkanDescriptorDataBinder& BindImage(UInt32 binding, const VulkanSampler* pSampler, const VulkanTexture* pTexture);

		bool Bind(VulkanDescriptorSet& set);
	private:
		void OverWriteData(VulkanDescriptorSet& set);
	private:
		VulkanDescriptorPool&        pool;
		VulkanDescriptorSetLayout&   layout;
		UInt32                       offset = 0;
		vector<VkWriteDescriptorSet> writes;

		eastl::array<VkDescriptorBufferInfo, 10> bufferInfos;
		UInt32                                   currentBufferIndex = 0;
		eastl::array<VkDescriptorImageInfo, 10>  imageInfos;
		UInt32                                   currentImageIndex = 0;
	};

}