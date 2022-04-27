#include "StdAfx.h"
#include "VulkanContext.h"

#include "Platform/OS.h"
#include "Memory/Memory.h"

#include "VulkanCommand.h"
#include "VulkanQueue.h"
#include "VulkanDescriptor.h"
#include "VulkanSynchronizePrimitive.h"
#include "VulkanFrameBuffer.h"
#include "VulkanTexture.h"
#include "VulkanQueryPool.h"

#include <eastl/array.h>

namespace SG
{

	VulkanContext::VulkanContext()
		:instance(), device(instance.physicalDevice)
	{
#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		// create vma allocator
		VmaAllocatorCreateInfo allocatorCI = {};
		allocatorCI.instance = instance.instance;
		allocatorCI.physicalDevice = instance.physicalDevice;
		allocatorCI.device = device.logicalDevice;
		allocatorCI.vulkanApiVersion = VK_MAKE_VERSION(1, 3, 0);

		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
		vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
		vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
		vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
		vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
		vulkanFunctions.vkCreateImage = vkCreateImage;
		vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
		vulkanFunctions.vkDestroyImage = vkDestroyImage;
		vulkanFunctions.vkFreeMemory = vkFreeMemory;
		vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
		vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
		vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
		vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
		vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
		vulkanFunctions.vkMapMemory = vkMapMemory;
		vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
		vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
		vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
		vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;

		allocatorCI.pVulkanFunctions = &vulkanFunctions;
		allocatorCI.pAllocationCallbacks = nullptr;
		vmaCreateAllocator(&allocatorCI, &vmaAllocator);

		SG_LOG_INFO("VmaAllocator initialized successfully!");
#endif
		pSwapchain = New(VulkanSwapchain, *this);

		pGraphicQueue  = device.GetQueue(EQueueType::eGraphic);
		pComputeQueue  = device.GetQueue(EQueueType::eCompute);
		pTransferQueue = device.GetQueue(EQueueType::eTransfer);

		Window* pMainWindow = OperatingSystem::GetMainWindow();
		pSwapchain->CreateOrRecreate(pMainWindow->GetWidth(), pMainWindow->GetHeight());

		CreateDefaultResource();

		colorRts.resize(pSwapchain->imageCount);
		for (UInt32 i = 0; i < colorRts.size(); ++i)
			colorRts[i] = pSwapchain->GetRenderTarget(i);

		TextureCreateDesc depthRtCI;
		depthRtCI.width = colorRts[0]->GetWidth();
		depthRtCI.height = colorRts[0]->GetHeight();
		depthRtCI.depth = colorRts[0]->GetDepth();
		depthRtCI.array = 1;
		depthRtCI.mipLevel = 1;
		depthRtCI.format = EImageFormat::eUnorm_D24_uint_S8;
		depthRtCI.sample = ESampleCount::eSample_1;
		depthRtCI.type  = EImageType::e2D;
		depthRtCI.usage = EImageUsage::efDepth_Stencil;
		depthRtCI.initLayout = EImageLayout::eUndefined;
		depthRtCI.memoryFlag = EGPUMemoryFlag::efDedicated_Memory;

		depthRt = VulkanRenderTarget::Create(*this, depthRtCI, true);

		// create command buffer
		commandBuffers.resize(pSwapchain->imageCount);
		for (auto& pCmdBuf : commandBuffers)
			graphicCommandPool->AllocateCommandBuffer(pCmdBuf);

		//computeCommandPool->AllocateCommandBuffer(computeCmdBuffer);
	}

	VulkanContext::~VulkanContext()
	{
		pSwapchain->CleanUp();
		Delete(pSwapchain);

		DestroyDefaultResource();
		Delete(depthRt);

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		vmaDestroyAllocator(vmaAllocator);
#endif
	}

	void VulkanContext::WindowResize()
	{
		pGraphicQueue->WaitIdle();

		Delete(depthRt);

		Window* pMainWindow = OperatingSystem::GetMainWindow();
		pSwapchain->CreateOrRecreate(pMainWindow->GetWidth(), pMainWindow->GetHeight());
		for (UInt32 i = 0; i < colorRts.size(); ++i)
			colorRts[i] = pSwapchain->GetRenderTarget(i); // this will new a VulkanRenderTarget

		TextureCreateDesc depthRtCI;
		depthRtCI.width = colorRts[0]->GetWidth();
		depthRtCI.height = colorRts[0]->GetHeight();
		depthRtCI.depth = colorRts[0]->GetDepth();
		depthRtCI.array = 1;
		depthRtCI.mipLevel = 1;
		depthRtCI.format = EImageFormat::eUnorm_D24_uint_S8;
		depthRtCI.sample = ESampleCount::eSample_1;
		depthRtCI.type = EImageType::e2D;
		depthRtCI.usage = EImageUsage::efDepth_Stencil;
		depthRtCI.initLayout = EImageLayout::eUndefined;

		depthRt = VulkanRenderTarget::Create(*this, depthRtCI, true);

		for (auto& pCmdBuf : commandBuffers)
		{
			graphicCommandPool->FreeCommandBuffer(pCmdBuf);
			graphicCommandPool->AllocateCommandBuffer(pCmdBuf);
		}
	}

	void VulkanContext::CreateDefaultResource()
	{
		// create a default command pool to allocate commands to graphic queue.
		graphicCommandPool = VulkanCommandPool::Create(device, VK_QUEUE_GRAPHICS_BIT);
		if (!graphicCommandPool)
			SG_LOG_ERROR("Failed to create default graphic command pool!");

		// create a default command pool to allocate commands to transfer queue.
		if (device.queueFamilyIndices.transfer == device.queueFamilyIndices.graphics)
			transferCommandPool = graphicCommandPool;
		else
		{
			transferCommandPool = VulkanCommandPool::Create(device, VK_QUEUE_TRANSFER_BIT);
			if (!transferCommandPool)
				SG_LOG_ERROR("Failed to create default transfer command pool!");
		}

		// create a default command pool to allocate commands to compute queue.
		if (device.queueFamilyIndices.compute == device.queueFamilyIndices.graphics)
			computeCommandPool = graphicCommandPool;
		else
		{
			computeCommandPool = VulkanCommandPool::Create(device, VK_QUEUE_COMPUTE_BIT);
			if (!computeCommandPool)
				SG_LOG_ERROR("Failed to create default compute command pool!");
		}

		pDefaultDescriptorPool = VulkanDescriptorPool::Builder()
			.AddPoolElement(EDescriptorType::eUniform_Buffer, 1000)
			.AddPoolElement(EDescriptorType::eStorage_Buffer, 1000)
			.AddPoolElement(EDescriptorType::eSampler, 1000)
			.AddPoolElement(EDescriptorType::eCombine_Image_Sampler, 1000)
			.AddPoolElement(EDescriptorType::eInput_Attachment, 1000)
			.AddPoolElement(EDescriptorType::eSampled_Image, 1000)
			.AddPoolElement(EDescriptorType::eStorage_Image, 1000)
			.AddPoolElement(EDescriptorType::eUniform_Buffer_Dynamic, 1000)
			.AddPoolElement(EDescriptorType::eStorage_Buffer_Dynamic, 1000)
			.SetMaxSets(9 * 1000)
			.Build(device);

		if (!pDefaultDescriptorPool)
			SG_LOG_ERROR("Failed to create default descriptor pool!");

		pFences.resize(pSwapchain->imageCount);
		for (Size i = 0; i < pSwapchain->imageCount; ++i)
		{
			VulkanFence** ppFence = &pFences[i];
			*ppFence = VulkanFence::Create(device, true);
		}

		pComputeCompleteSemaphore = VulkanSemaphore::Create(device);
		pRenderCompleteSemaphore = VulkanSemaphore::Create(device);
		pPresentCompleteSemaphore = VulkanSemaphore::Create(device);

		pComputeSyncFences.resize(pSwapchain->imageCount);
		for (Size i = 0; i < pSwapchain->imageCount; ++i)
		{
			VulkanFence** ppFence = &pComputeSyncFences[i];
			*ppFence = VulkanFence::Create(device, true);
		}

		EPipelineStageQueryType types =
			EPipelineStageQueryType::efInput_Assembly_Vertices |
			EPipelineStageQueryType::efInput_Assembly_Primitives |
			EPipelineStageQueryType::efVertex_Shader_Invocations |
			EPipelineStageQueryType::efClipping_Invocations |
			EPipelineStageQueryType::efClipping_Primitives |
			EPipelineStageQueryType::efFragment_Shader_Invocations;
		pPipelineStatisticsQueryPool = VulkanQueryPool::Create(device, ERenderQueryType::ePipeline_Statistics, types);
		pTimeStampQueryPool = VulkanQueryPool::Create(device, ERenderQueryType::eTimeStamp, 6);
	}

	void VulkanContext::DestroyDefaultResource()
	{
		Delete(pTimeStampQueryPool);
		Delete(pPipelineStatisticsQueryPool);

		for (auto* pFence : pComputeSyncFences)
			Delete(pFence);
		Delete(pComputeCompleteSemaphore);

		Delete(pRenderCompleteSemaphore);
		Delete(pPresentCompleteSemaphore);
		for (auto* pFence : pFences)
			Delete(pFence);

		Delete(pDefaultDescriptorPool);
		if (computeCommandPool && device.queueFamilyIndices.graphics != device.queueFamilyIndices.compute)
			Delete(computeCommandPool);
		if (transferCommandPool && device.queueFamilyIndices.graphics != device.queueFamilyIndices.transfer)
			Delete(transferCommandPool);
		if (graphicCommandPool)
			Delete(graphicCommandPool);
	}

}