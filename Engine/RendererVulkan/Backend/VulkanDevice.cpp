#include "StdAfx.h"
#include "VulkanDevice.h"

#include "System/ILogger.h"

#include "VulkanInstance.h"

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
		if (logicalDevice != VK_NULL_HANDLE)
			DestroyLogicalDevice();
		//if (defaultCommandPool != VK_NULL_HANDLE)
		//	vkDestroyCommandPool(logicalDevice, defaultCommandPool, nullptr);
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

		//if (!bAllDeviceExtensionSupported)
		//	SG_LOG_WARN("Extensions in physical device do not include the others in instance");

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
		//defaultCommandPool = CreateCommandPool(queueFamilyIndices.graphics);
		//if (defaultCommandPool == VK_NULL_HANDLE)
		//{
		//	SG_LOG_ERROR("Failed to create default command pool!");
		//	return false;
		//}

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

}