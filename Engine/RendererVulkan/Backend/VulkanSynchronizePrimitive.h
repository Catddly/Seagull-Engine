#pragma once

#include "Render/Synchronize.h"
#include "VulkanDevice.h"
 
#include "volk.h"
#include <stdint.h>

namespace SG
{

	class VulkanSemaphore
	{
	public:
		VulkanSemaphore(VulkanDevice& d, bool bTimeLine);
		~VulkanSemaphore();

		static VulkanSemaphore* Create(VulkanDevice& d, bool bTimeLine = false);

		void Signal();
	private:
		friend class VulkanQueue;
		friend class VulkanSwapchain;

		VulkanDevice& device;
		VkSemaphore   semaphore;
	};

	class VulkanFence
	{
	public:
		VulkanFence(VulkanDevice& d, bool bSignaled);
		~VulkanFence();

		void Reset();
		void Wait(UInt64 timeOutNs = UINT64_MAX);
		void WaitAndReset(UInt64 timeOutNs = UINT64_MAX);

		static VulkanFence* Create(VulkanDevice& d, bool bSignaled = false);
	private:
		friend class VulkanQueue;

		VulkanDevice& device;
		VkFence       fence;
	};
}