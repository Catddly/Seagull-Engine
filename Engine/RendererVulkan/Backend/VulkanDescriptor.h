#pragma once

#include "Base/BasicTypes.h"
#include "Render/Descriptor.h"

#include "VulkanDevice.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"
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

		VkDescriptorSetLayout& NativeHandle() { return descriptorSetLayout; }

		class Builder
		{
		public:
			Builder(VulkanDevice& d) : device(d) {}

			Builder& AddBinding(EDescriptorType type, EShaderStage stage, UInt32 binding, UInt32 count);
			VulkanDescriptorSetLayout* Build();
		private:
			VulkanDevice& device;
			eastl::unordered_map<UInt32, VkDescriptorSetLayoutBinding> bindingsMap;
		};
	private:
		friend class VulkanDescriptorDataBinder;

		VulkanDevice&         device;
		VkDescriptorSetLayout descriptorSetLayout;
		eastl::unordered_map<UInt32, VkDescriptorSetLayoutBinding> bindingsMap;
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

		bool Bind(VkDescriptorSet& set);
	private:
		void OverWriteData(VkDescriptorSet& set);
	private:
		VulkanDescriptorPool&        pool;
		VulkanDescriptorSetLayout&   layout;
		vector<VkWriteDescriptorSet> writes;

		vector<VkDescriptorBufferInfo> bufferInfos;
		vector<VkDescriptorImageInfo> imageInfos;
	};

}