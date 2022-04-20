#include "StdAfx.h"
#include "VulkanDescriptor.h"

#include "Defs/Defs.h"
#include "Memory/Memory.h"

#include "VulkanConfig.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

#include "RendererVulkan/Utils/VkConvert.h"

#include "Math/MathBasic.h"

namespace SG
{
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanDescriptorPool
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::AddPoolElement(EDescriptorType type, UInt32 count)
	{
		VkDescriptorPoolSize size;
		size.type = ToVkDescriptorType(type);
		size.descriptorCount = count;
		poolSizes.emplace_back(size);
		return *this;
	}

	VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::SetMaxSets(UInt32 max)
	{
		SG_ASSERT(max >= 1 && "Number of set should at least be one!");
		maxSets = max;
		return *this;
	}

	VulkanDescriptorPool* VulkanDescriptorPool::Builder::Build(VulkanDevice& d)
	{
		if (poolSizes.size() == 0)
		{
			SG_LOG_WARN("Please add at least one pool element first!");
			return nullptr;
		}

		return Memory::New<VulkanDescriptorPool>(d, poolSizes, maxSets);
	}

	VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice& d, const vector<VkDescriptorPoolSize>& poolSizes, UInt32 maxSets)
		:device(d)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.poolSizeCount = static_cast<UInt32>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;

		VK_CHECK(vkCreateDescriptorPool(device.logicalDevice, &descriptorPoolInfo, nullptr, &pool),
			SG_LOG_ERROR("Failed to create descriptor pool!"););
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		vkDestroyDescriptorPool(device.logicalDevice, pool, nullptr);
	}

	bool VulkanDescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout& layout, VkDescriptorSet& set)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VK_CHECK(vkAllocateDescriptorSets(device.logicalDevice, &allocInfo, &set),
			SG_LOG_ERROR("Failed to allocate descriptor set!"); return false;);
		return true;
	}

	void VulkanDescriptorPool::FreeDescriptorSet(VkDescriptorSet& set)
	{
		vkFreeDescriptorSets(device.logicalDevice, pool, 1, &set);
	}

	void VulkanDescriptorPool::Reset()
	{
		vkResetDescriptorPool(device.logicalDevice, pool, 0);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanDescriptorSet
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice& d, const eastl::unordered_map<UInt32, VkDescriptorSetLayoutBinding>& layouts)
		:device(d), bindingsMap(layouts)
	{
		vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto& kv : bindingsMap)
			bindings.emplace_back(kv.second);

		VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
		descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayout.pNext = nullptr;
		descriptorLayout.bindingCount = static_cast<UInt32>(bindings.size());
		descriptorLayout.pBindings = bindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout),
			SG_LOG_ERROR("Failed to create descriptor set layout!"););
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(device.logicalDevice, descriptorSetLayout, nullptr);
	}

	VulkanDescriptorSetLayout::Builder& VulkanDescriptorSetLayout::Builder::AddBinding(EDescriptorType type, EShaderStage stage, UInt32 binding, UInt32 count)
	{
		if (bindingsMap.count(binding) != 0)
		{
			SG_LOG_WARN("Binding %d already in use!", binding); // do nothing
			return *this;
		}

		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.descriptorType = ToVkDescriptorType(type);
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = ToVkShaderStageFlags(stage); // binding stage
		layoutBinding.pImmutableSamplers = nullptr;

		bindingsMap[binding] = layoutBinding;
		return *this;
	}

	RefPtr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayout::Builder::Build()
	{
		return MakeRef<VulkanDescriptorSetLayout>(device, eastl::move(bindingsMap));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanDescriptorDataBinder
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanDescriptorDataBinder::VulkanDescriptorDataBinder(VulkanDescriptorPool& p, VulkanDescriptorSetLayout& l)
		:pool(p), layout(l)
	{
	}

	VulkanDescriptorDataBinder& VulkanDescriptorDataBinder::BindBuffer(UInt32 binding, const VulkanBuffer* pBuf)
	{
		if (layout.bindingsMap.count(binding) == 0)
		{
			SG_LOG_WARN("No binding %d in descriptorSet layout!");
			return *this;
		}

		auto& bindingInfo = layout.bindingsMap[binding];

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = pBuf->buffer;
		bufferInfo.range = pBuf->size;
		bufferInfo.offset = 0;
		offset += pBuf->size;

		bufferInfos[currentBufferIndex] = eastl::move(bufferInfo);

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingInfo.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = &bufferInfos[currentBufferIndex++];
		write.dstArrayElement = 0;
		write.descriptorCount = bindingInfo.descriptorCount;

		writes.push_back(eastl::move(write));
		return *this;
	}

	VulkanDescriptorDataBinder& VulkanDescriptorDataBinder::BindImage(UInt32 binding, const VulkanSampler* pSampler, const VulkanTexture* pTexture)
	{
		if (layout.bindingsMap.count(binding) == 0)
		{
			SG_LOG_WARN("No binding %d in descriptorSet layout!");
			return *this;
		}

		auto& bindingInfo = layout.bindingsMap[binding];

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.sampler = pSampler->sampler;
		imageInfo.imageView = pTexture->imageView;
		imageInfo.imageLayout = pTexture->currLayout;

		imageInfos[currentImageIndex] = eastl::move(imageInfo);

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingInfo.descriptorType;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.pImageInfo = &imageInfos[currentImageIndex++];
		write.descriptorCount = bindingInfo.descriptorCount;

		writes.emplace_back(eastl::move(write));
		return *this;
	}

	bool VulkanDescriptorDataBinder::Bind(VulkanDescriptorSet& set)
	{
		if (!pool.AllocateDescriptorSet(layout.descriptorSetLayout, set.set))
			return false;
		OverWriteData(set);
		return true;
	}

	void VulkanDescriptorDataBinder::OverWriteData(VulkanDescriptorSet& set)
	{
		for (auto& w : writes) // all the data bind to this set
			w.dstSet = set.set;
		vkUpdateDescriptorSets(pool.device.logicalDevice, static_cast<UInt32>(writes.size()), writes.data(), 0, nullptr);
	}

}