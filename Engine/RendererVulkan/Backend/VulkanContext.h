#pragma once

#include "Defs/Defs.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommand.h"
#include "VulkanAllocator.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"

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
	class VulkanQueue;

	class VulkanPipelineSignature;
	class VulkanShader;

	class VulkanQueryPool;

	class VulkanContext
	{
	public:
		VulkanContext();
		~VulkanContext();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanContext);

		VulkanInstance  instance;
		VulkanDevice    device;

		VulkanSwapchain* pSwapchain;

		VulkanCommandPool* pGraphicCommandPool;
		VulkanCommandPool* pComputeCommandPool;
		VulkanCommandPool* pTransferCommandPool;

		vector<VulkanCommandBuffer> commandBuffers;

		VulkanQueue* pGraphicQueue;
		VulkanQueue* pComputeQueue;
		VulkanQueue* pTransferQueue;

		// Compute Sync
		VulkanSemaphore* pComputeCompleteSemaphore;
		vector<VulkanFence*> pComputeSyncFences;

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

		VulkanQueryPool* pPipelineStatisticsQueryPool;
		VulkanQueryPool* pTimeStampQueryPool;
		// for multi-texture bind test
		VulkanPipelineSignature* pTempPipelineSignature = nullptr;
		RefPtr<VulkanShader> pShader = nullptr;

		void WindowResize();
	private:
		void CreateDefaultResource();
		void DestroyDefaultResource();
	};

}