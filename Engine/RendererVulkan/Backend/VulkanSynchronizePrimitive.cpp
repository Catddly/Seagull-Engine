#include "StdAfx.h"
#include "VulkanSynchronizePrimitive.h"

#include "VulkanConfig.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

namespace SG
{

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanSemaphore
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanSemaphore::VulkanSemaphore(VulkanDevice& d)
		:device(d)
	{
		VkSemaphoreCreateInfo semaphoreCI = {};
		semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCI.pNext = nullptr;
		VK_CHECK(vkCreateSemaphore(device.logicalDevice, &semaphoreCI, nullptr, &semaphore),
			SG_LOG_ERROR("Failed to create vulkan semaphore!"););
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		vkDestroySemaphore(device.logicalDevice, semaphore, nullptr);
	}

	VulkanSemaphore* VulkanSemaphore::Create(VulkanDevice& d)
	{
		return Memory::New<VulkanSemaphore>(d);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanFence
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanFence::VulkanFence(VulkanDevice& d, bool bSignaled)
		:device(d)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// create in signaled state so we don't wait on first render of each command buffer.
		if (bSignaled)
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VK_CHECK(vkCreateFence(device.logicalDevice, &fenceCreateInfo, nullptr, &fence),
			SG_LOG_ERROR("Failed to create vulkan fence"););
	}

	VulkanFence::~VulkanFence()
	{
		vkDestroyFence(device.logicalDevice, fence, nullptr);
	}

	void VulkanFence::Reset()
	{
		vkResetFences(device.logicalDevice, 1, &fence);
	}

	void VulkanFence::WaitAndReset(UInt64 timeOut)
	{
		vkWaitForFences(device.logicalDevice, 1, &fence, VK_TRUE, timeOut);
		Reset();
	}

	VulkanFence* VulkanFence::Create(VulkanDevice& d, bool bSignaled)
	{
		return Memory::New<VulkanFence>(d, bSignaled);
	}

}