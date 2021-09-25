#include "StdAfx.h"
#include "VulkanDevice.h"

#include "System/ILogger.h"

#include "VulkanInstance.h"
#include "VulkanSwapchain.h"
#include "RendererVulkan/Utils/VkConvert.h"

#include "Stl/vector.h"

namespace SG
{

	VulkanDevice::VulkanDevice(VkPhysicalDevice device)
		:physicalDevice(device)
	{
		// get all the extensions supported by device
		UInt32 extCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			supportedExtensions.resize(extCount);
			vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, supportedExtensions.data());
		}

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
	}

	VulkanDevice::~VulkanDevice()
	{
		if (defaultCommandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(logicalDevice, defaultCommandPool, nullptr);
		if (logicalDevice != VK_NULL_HANDLE)
			DestroyLogicalDevice();
	}

	bool VulkanDevice::CreateLogicalDevice(void* pNext)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

		const float DEFAULT_QUEUE_PRIORITY = 0.0f;
		// graphic queue
		int graphics = FetchQueueFamilyIndicies(VK_QUEUE_GRAPHICS_BIT);
		if (graphics != -1)
		{
			queueFamilyIndices.graphics = graphics;
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &DEFAULT_QUEUE_PRIORITY;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.graphics = VK_NULL_HANDLE;
		}

		// compute queue
		int compute = FetchQueueFamilyIndicies(VK_QUEUE_COMPUTE_BIT);
		if (compute != -1 && compute != graphics)
		{
			queueFamilyIndices.compute = compute;
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &DEFAULT_QUEUE_PRIORITY;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.compute = queueFamilyIndices.graphics;
		}

		// transfer queue
		int transfer = FetchQueueFamilyIndicies(VK_QUEUE_TRANSFER_BIT);
		if (transfer != -1 && transfer != graphics)
		{
			queueFamilyIndices.transfer = transfer;
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &DEFAULT_QUEUE_PRIORITY;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.transfer = queueFamilyIndices.graphics;
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		// to support swapchain
		vector<const char*> deviceExtensions = {};
		if (SupportExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		if (SupportExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
			deviceExtensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = (UInt32)queueCreateInfos.size();
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = (UInt32)deviceExtensions.size();;
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		createInfo.enabledLayerCount = 0;
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		vector<const char*> layers;
		layers.emplace_back("VK_LAYER_KHRONOS_validation");

		createInfo.enabledLayerCount = (UInt32)layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
#endif
		
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
		if (pNext) 
		{
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.features = deviceFeatures;
			physicalDeviceFeatures2.pNext = pNext;
			createInfo.pEnabledFeatures = nullptr;
			createInfo.pNext = &physicalDeviceFeatures2;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create logical device!");
			return false;
		}

		// create a default command pool to allocate commands to graphic queue.
		defaultCommandPool = CreateCommandPool(queueFamilyIndices.graphics);
		if (defaultCommandPool == VK_NULL_HANDLE)
		{
			SG_LOG_ERROR("Failed to create default command pool!");
			return false;
		}

		return true;
	}

	void VulkanDevice::DestroyLogicalDevice()
	{
		vkDestroyDevice(logicalDevice, nullptr);
	}

	VkCommandPool VulkanDevice::CreateCommandPool(UInt32 queueFamilyIndices, VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndices;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;
		if (vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &cmdPool) != VK_SUCCESS)
			return VK_NULL_HANDLE;
		return cmdPool;
	}

	bool VulkanDevice::AllocateCommandBuffers(vector<VkCommandBuffer>& commandBuffers)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = defaultCommandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = (UInt32)commandBuffers.size();
		
		if (vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to allocate command buffer");
			return false;
		}
		return true;
	}

	void VulkanDevice::FreeCommandBuffers(vector<VkCommandBuffer>& commandBuffers)
	{
		vkFreeCommandBuffers(logicalDevice, defaultCommandPool, (UInt32)commandBuffers.size(), commandBuffers.data());
	}

	bool VulkanDevice::CreateRenderTarget(VulkanRenderTarget* rt)
	{
		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = rt->type;
		imageCI.format = rt->format;
		imageCI.extent = { rt->width, rt->height, rt->depth };
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = rt->array;
		imageCI.samples = rt->sample;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = rt->usage;

		if (vkCreateImage(logicalDevice, &imageCI, nullptr, &rt->image) != VK_NULL_HANDLE)
		{
			SG_LOG_ERROR("Failed to create render targets' image!");
			return false;
		}

		VkMemoryRequirements memReqs = {};
		vkGetImageMemoryRequirements(logicalDevice, rt->image, &memReqs);

		VkMemoryAllocateInfo memAllloc = {};
		memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllloc.allocationSize = memReqs.size;
		memAllloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkAllocateMemory(logicalDevice, &memAllloc, nullptr, &rt->memory);
		vkBindImageMemory(logicalDevice, rt->image, rt->memory, 0);

		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.viewType = ToVkImageViewType(rt->type, rt->array);
		imageViewCI.image = rt->image;
		imageViewCI.format = rt->format;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = rt->array;

		if (rt->usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			// stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
			if (rt->format >= VK_FORMAT_D16_UNORM_S8_UINT) {
				imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

		if (vkCreateImageView(logicalDevice, &imageViewCI, nullptr, &rt->imageView) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create render targets' image view!");
			return false;
		}

		return true;
	}

	void VulkanDevice::DestroyRenderTarget(VulkanRenderTarget* rt)
	{
		vkDestroyImageView(logicalDevice, rt->imageView, nullptr);
		vkDestroyImage(logicalDevice, rt->image, nullptr);
		vkFreeMemory(logicalDevice, rt->memory, nullptr);
	}

	VulkanQueue VulkanDevice::GetQueue(EQueueType type) const
	{
		VulkanQueue queue;
		switch (type)
		{
		case SG::EQueueType::eNull: SG_LOG_ERROR("Wrong queue type!"); break;
		case SG::EQueueType::eGraphic:  
			vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphics, 0, &queue.handle); 
			queue.familyIndex = queueFamilyIndices.graphics;
			break;
		case SG::EQueueType::eCompute:
			vkGetDeviceQueue(logicalDevice, queueFamilyIndices.compute, 0, &queue.handle);
			queue.familyIndex = queueFamilyIndices.compute;
			break;
		case SG::EQueueType::eTransfer: 
			vkGetDeviceQueue(logicalDevice, queueFamilyIndices.transfer, 0, &queue.handle);
			queue.familyIndex = queueFamilyIndices.transfer;
			break;
		default: SG_LOG_ERROR("Unknown queue type!"); break;
		}
		queue.type = type;
		queue.priority = EQueuePriority::eNormal;
		return eastl::move(queue);
	}

	bool VulkanDevice::SupportExtension(const string& extension)
	{
		for (auto beg = supportedExtensions.begin(); beg != supportedExtensions.end(); beg++)
		{
			if (beg->extensionName == extension)
				return true;
		}
		return false;
	}

	int VulkanDevice::FetchQueueFamilyIndicies(VkQueueFlagBits flags)
	{
		// find a compute queue
		UInt32 i = 0;
		if (flags & VK_QUEUE_COMPUTE_BIT)
		{
			for (const auto& e : queueFamilyProperties)
			{
				if ((e.queueFlags & flags) && ((e.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) // have COMPUTE_BIT but bot have the GRAPHIC_BIT
				{
					return i;
				}
				++i;
			}
		}

		// find a transfer queue
		i = 0;
		if (flags & VK_QUEUE_TRANSFER_BIT)
		{
			for (const auto& e : queueFamilyProperties)
			{
				if ((e.queueFlags & flags) && ((e.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
					&& ((e.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) // have TRANSFER_BIT but bot have the GRAPHIC_BIT
				{
					return i;
				}
				++i;
			}
		}

		// find a graphics queue
		i = 0;
		for (const auto& e : queueFamilyProperties)
		{
			if (e.queueFlags & flags) // have TRANSFER_BIT but bot have the GRAPHIC_BIT
			{
				return i;
			}
			++i;
		}

		return -1;
	}

	SG::UInt32 VulkanDevice::GetMemoryType(UInt32 typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (memTypeFound)
					{
						*memTypeFound = true;
					}
					return i;
				}
			}
			typeBits >>= 1;
		}

		if (memTypeFound)
		{
			*memTypeFound = false;
			return 0;
		}
		else
		{
			SG_LOG_ERROR("Failed to find device memory type!");
			SG_ASSERT(false);
		}

		return 0;
	}

}