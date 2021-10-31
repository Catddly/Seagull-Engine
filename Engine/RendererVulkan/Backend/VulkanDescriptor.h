#pragma once

#include "Base/BasicTypes.h"
#include "Render/Buffer.h"

#include "VulkanDevice.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"

namespace SG
{

	class VulkanDescriptor
	{
	public:

	private:
		VkDescriptorSet descriptorSet;
	};

	class VulkanDescriptorPool
	{
	public:
		VulkanDescriptorPool(VulkanDevice& d);
		~VulkanDescriptorPool();

		class Builder
		{
		public:
			Builder& AddPoolElement(EBufferType type, UInt32 count);
			VulkanDescriptorPool* Build(VulkanDevice& d);
		private:
			VulkanDevice& device;
			vector<VkDescriptorPoolSize> poolSizes;
		};
	private:
		friend class Builder;
		VkDescriptorPool pool;
	};

	//VkDescriptorPool pool;
	//VkDescriptorPoolSize typeCounts[1];
	//typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//typeCounts[0].descriptorCount = 1;

	//VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	//descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//descriptorPoolInfo.pNext = nullptr;
	//descriptorPoolInfo.poolSizeCount = 1;
	//descriptorPoolInfo.pPoolSizes = typeCounts;
	//descriptorPoolInfo.maxSets = 1;

}