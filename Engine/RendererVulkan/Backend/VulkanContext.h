#pragma once

#include "Defs/Defs.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommand.h"

#include "Stl/vector.h"

#define VK_API_VERSION_MAJOR(version) (((uint32_t)(version) >> 22) & 0x7FU)
#define VK_API_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3FFU)
#define VK_API_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)
#include "vma/vk_mem_alloc.h"

namespace SG
{

	class VulkanDescriptorPool;
	class VulkanCommandPool;
	class VulkanRenderPass;
	class VulkanSemaphore;
	class VulkanFence;
	class VulkanRenderPass;
	class VulkanRenderTarget;
	class VulkanFrameBuffer;

	class VulkanContext
	{
	public:
		VulkanContext();
		~VulkanContext();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanContext);

		VulkanInstance  instance;
		VulkanDevice    device;
		VulkanSwapchain swapchain;

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

		VmaAllocator vmaAllocator;

		void WindowResize();
	private:
		void CreateDefaultResource();
		void DestroyDefaultResource();
	};

}