#include "StdAfx.h"
#include "VulkanDevice.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

#include "VulkanConfig.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"
#include "VulkanCommand.h"
#include "VulkanSynchronizePrimitive.h"
#include "RendererVulkan/Utils/VkConvert.h"

#include "Stl/vector.h"
#include <eastl/array.h>

namespace SG
{

	VulkanDevice::VulkanDevice(VkPhysicalDevice& device)
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

		VkPhysicalDeviceProperties physicalDeviceProps;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);
		physicalDeviceLimits = physicalDeviceProps.limits;
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

		CreateLogicalDevice(nullptr);

		volkLoadDevice(logicalDevice);
	}

	VulkanDevice::~VulkanDevice()
	{
		if (logicalDevice != VK_NULL_HANDLE)
			DestroyLogicalDevice();
	}

	void VulkanDevice::WaitIdle() const
	{
		vkDeviceWaitIdle(logicalDevice);
	}

	bool VulkanDevice::CreateLogicalDevice(void* pNext)
	{
		vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

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
		if (physicalDeviceFeatures.samplerAnisotropy) // if device support anisotropy, enable it.
			deviceFeatures.samplerAnisotropy = VK_TRUE;

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
		return true;
	}

	void VulkanDevice::DestroyLogicalDevice()
	{
		vkDestroyDevice(logicalDevice, nullptr);
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
		queue.type     = type;
		queue.priority = EQueuePriority::eNormal;
		return queue;
	}

	VkDescriptorSet VulkanDevice::AllocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool)
	{
		VkDescriptorSet descriptorSet;
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VK_CHECK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet),
			SG_LOG_ERROR("Failed to allocate descriptor set!"); return VK_NULL_HANDLE; );
		return descriptorSet;
	}

	void VulkanDevice::FreeDescriptorSet(VkDescriptorSet set, VkDescriptorPool pool)
	{
		vkFreeDescriptorSets(logicalDevice, pool, 1, &set);
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanQueue
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanQueue::SubmitCommands(VulkanCommandBuffer* pCmdBuf, VulkanSemaphore* pSignalSemaphore, VulkanSemaphore* pWaitSemaphore, VulkanFence* fence)
	{
		if (pCmdBuf->queueFamilyIndex != familyIndex) // check if the command is submitted to the right queue
		{
			SG_LOG_ERROR("Vulkan command buffer had been submit to the wrong queue! (Submit To: %s)", 
				(type == EQueueType::eGraphic) ? "Graphic" : (type == EQueueType::eCompute ? "Compute" : "Transfer"));
			return false;
		}

		// pipeline stage at which the queue submission will wait (via pWaitSemaphores)
		// the submit info structure specifies a command buffer queue submission batch
		VkPipelineStageFlags waitStageMask = {};
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		if (pSignalSemaphore && pWaitSemaphore)
		{
			waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		submitInfo.pWaitDstStageMask = &waitStageMask;

		if (pSignalSemaphore)
		{
			submitInfo.pWaitSemaphores = &pWaitSemaphore->semaphore;
			submitInfo.waitSemaphoreCount = 1;
		}
		if (pWaitSemaphore)
		{
			submitInfo.pSignalSemaphores = &pSignalSemaphore->semaphore;
			submitInfo.signalSemaphoreCount = 1;
		}
		submitInfo.pCommandBuffers = &pCmdBuf->commandBuffer;
		submitInfo.commandBufferCount = 1;

		VK_CHECK(vkQueueSubmit(handle, 1, &submitInfo, fence ? fence->fence : nullptr),
			SG_LOG_ERROR("Failed to submit render commands to queue!");
			return false;);
		return true;
	}

	void VulkanQueue::WaitIdle() const
	{
		vkQueueWaitIdle(handle);
	}

}