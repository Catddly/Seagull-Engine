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

	VulkanSemaphore::VulkanSemaphore(VulkanDevice& d, bool bTimeLine)
		:device(d)
	{
		VkSemaphoreCreateInfo semaphoreCI = {};
		semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCI.pNext = nullptr;
		VkSemaphoreTypeCreateInfo typeCI = {};
		if (bTimeLine)
		{
			typeCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			typeCI.initialValue = 0;
			typeCI.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
			semaphoreCI.pNext = &typeCI;
		}

		VK_CHECK(vkCreateSemaphore(device.logicalDevice, &semaphoreCI, nullptr, &semaphore),
			SG_LOG_ERROR("Failed to create vulkan semaphore!"););
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		vkDestroySemaphore(device.logicalDevice, semaphore, nullptr);
	}

	VulkanSemaphore* VulkanSemaphore::Create(VulkanDevice& d, bool bTimeLine)
	{
		return Memory::New<VulkanSemaphore>(d, bTimeLine);
	}

	void VulkanSemaphore::Signal()
	{
		VkSemaphoreSignalInfo signalInfo = {};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		signalInfo.value = 1;
		signalInfo.semaphore = semaphore;

		vkSignalSemaphore(device.logicalDevice, &signalInfo);
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

	void VulkanFence::Wait(UInt64 timeOutNs)
	{
		vkWaitForFences(device.logicalDevice, 1, &fence, VK_TRUE, timeOutNs);
	}

	void VulkanFence::WaitAndReset(UInt64 timeOutNs)
	{
		Wait(timeOutNs);
		Reset();
	}

	VulkanFence* VulkanFence::Create(VulkanDevice& d, bool bSignaled)
	{
		return Memory::New<VulkanFence>(d, bSignaled);
	}

}