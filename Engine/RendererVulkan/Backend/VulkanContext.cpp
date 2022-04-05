#include "StdAfx.h"
#include "VulkanContext.h"

#include "Platform/OS.h"
#include "Memory/Memory.h"

#include "VulkanCommand.h"
#include "VulkanDescriptor.h"
#include "VulkanSynchronizePrimitive.h"
#include "VulkanFrameBuffer.h"

#include <eastl/array.h>

namespace SG
{

	VulkanContext::VulkanContext()
		:instance(), device(instance.physicalDevice), swapchain(instance, device)
	{
		graphicQueue  = device.GetQueue(EQueueType::eGraphic);
		computeQueue  = device.GetQueue(EQueueType::eCompute);
		transferQueue = device.GetQueue(EQueueType::eTransfer);

		Window* pMainWindow = OperatingSystem::GetMainWindow();
		swapchain.CreateOrRecreate(pMainWindow->GetWidth(), pMainWindow->GetHeight());

		CreateDefaultResource();

		colorRts.resize(swapchain.imageCount);
		for (UInt32 i = 0; i < colorRts.size(); ++i)
			colorRts[i] = swapchain.GetRenderTarget(i);

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

		depthRt = VulkanRenderTarget::Create(device, depthRtCI, true);

		// create command buffer
		commandBuffers.resize(swapchain.imageCount);
		for (auto& pCmdBuf : commandBuffers)
			graphicCommandPool->AllocateCommandBuffer(pCmdBuf);

		computeCommandPool->AllocateCommandBuffer(computeCmdBuffer);
	}

	VulkanContext::~VulkanContext()
	{
		swapchain.CleanUp();
		DestroyDefaultResource();

		Memory::Delete(depthRt);
	}

	void VulkanContext::WindowResize()
	{
		graphicQueue.WaitIdle();

		Memory::Delete(depthRt);

		Window* pMainWindow = OperatingSystem::GetMainWindow();
		swapchain.CreateOrRecreate(pMainWindow->GetWidth(), pMainWindow->GetHeight());
		for (UInt32 i = 0; i < colorRts.size(); ++i)
			colorRts[i] = swapchain.GetRenderTarget(i); // this will new a VulkanRenderTarget

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

		depthRt = VulkanRenderTarget::Create(device, depthRtCI, true);

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

		pFences.resize(swapchain.imageCount);
		for (Size i = 0; i < swapchain.imageCount; ++i)
		{
			VulkanFence** ppFence = &pFences[i];
			*ppFence = VulkanFence::Create(device, true);
		}

		pComputeCompleteSemaphore = VulkanSemaphore::Create(device);
		pRenderCompleteSemaphore = VulkanSemaphore::Create(device);
		pPresentCompleteSemaphore = VulkanSemaphore::Create(device);

		pComputeSyncFence = VulkanFence::Create(device, true);
	}

	void VulkanContext::DestroyDefaultResource()
	{
		Memory::Delete(pComputeSyncFence);
		Memory::Delete(pComputeCompleteSemaphore);

		Memory::Delete(pRenderCompleteSemaphore);
		Memory::Delete(pPresentCompleteSemaphore);
		for (auto* pFence : pFences)
			Memory::Delete(pFence);

		Memory::Delete(pDefaultDescriptorPool);
		if (computeCommandPool && device.queueFamilyIndices.graphics != device.queueFamilyIndices.compute)
			Memory::Delete(computeCommandPool);
		if (transferCommandPool && device.queueFamilyIndices.graphics != device.queueFamilyIndices.transfer)
			Memory::Delete(transferCommandPool);
		if (graphicCommandPool)
			Memory::Delete(graphicCommandPool);
	}

}