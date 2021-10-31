#include "StdAfx.h"
#include "VulkanDescriptor.h"

namespace SG
{

	VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::AddPoolElement(EBufferType type, UInt32 count)
	{
		VkDescriptorPoolSize size;
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		size.descriptorCount = count;
		poolSizes.emplace_back(size);
		return *this;
	}

	SG::VulkanDescriptorPool* VulkanDescriptorPool::Builder::Build(VulkanDevice& d)
	{
		if (poolSizes.size() == 0)
			SG_LOG_WARN("Please add at least one pool element first!");
	}

	VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice& d)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.poolSizeCount = poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK(vkCreateDescriptorPool(device.logicalDevice, &descriptorPoolInfo, nullptr, &pool),
			SG_LOG_ERROR("Failed to create descriptor pool!"); return nullptr; );
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{

	}

}