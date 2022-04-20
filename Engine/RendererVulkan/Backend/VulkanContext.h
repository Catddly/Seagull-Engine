#pragma once

#include "Defs/Defs.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommand.h"
#include "VulkanAllocator.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanDescriptorPool;
	class VulkanCommandPool;
	class VulkanRenderPass;
	class VulkanSemaphore;
	class VulkanRenderTarget;
	class VulkanFence;
	class VulkanRenderPass;
	class VulkanFrameBuffer;

	class VulkanContext
	{
	public:
		VulkanContext();
		~VulkanContext();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanContext);

		VulkanInstance  instance;
		VulkanDevice    device;
		VulkanSwapchain* pSwapchain;

		VulkanCommandPool* graphicCommandPool;
		VulkanCommandPool* computeCommandPool;
		VulkanCommandPool* transferCommandPool;

		VulkanCommandBuffer computeCmdBuffer;
		vector<VulkanCommandBuffer> commandBuffers;

		VulkanQueue        graphicQueue;
		VulkanQueue        computeQueue;
		VulkanQueue        transferQueue;

		// Compute Sync
		VulkanSemaphore* pComputeCompleteSemaphore;
		VulkanFence*     pComputeSyncFence;

		// [GPU To GPU Synchronization]
		VulkanSemaphore* pRenderCompleteSemaphore;
		VulkanSemaphore* pPresentCompleteSemaphore;
		// [CPU To GPU Synchronization]
		vector<VulkanFence*> pFences;

		VulkanDescriptorPool* pDefaultDescriptorPool;

		vector<VulkanRenderTarget*> colorRts;
		VulkanRenderTarget*         depthRt;

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		VmaAllocator vmaAllocator;
#endif
		void WindowResize();
	private:
		void CreateDefaultResource();
		void DestroyDefaultResource();
	};

}