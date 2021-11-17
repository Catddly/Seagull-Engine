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
		:instance(), swapchain(instance, device), device(instance.physicalDevice)
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

		RenderTargetCreateDesc depthRtCI;
		depthRtCI.width = colorRts[0]->GetWidth();
		depthRtCI.height = colorRts[0]->GetHeight();
		depthRtCI.depth = colorRts[0]->GetDepth();
		depthRtCI.array = 1;
		depthRtCI.mipmap = 1;
		depthRtCI.format = EImageFormat::eUnorm_D24_uint_S8;
		depthRtCI.sample = ESampleCount::eSample_1;
		depthRtCI.type  = EImageType::e2D;
		depthRtCI.usage = EImageUsage::efDepth_Stencil;

		depthRt = VulkanRenderTarget::Create(device, depthRtCI);

		// create command buffer
		commandBuffers.resize(swapchain.imageCount);
		for (auto& pCmdBuf : commandBuffers)
			graphicCommandPool->AllocateCommandBuffer(pCmdBuf);
	}

	VulkanContext::~VulkanContext()
	{
		swapchain.CleanUp();
		DestroyDefaultResource();

		Memory::Delete(depthRt);
	}

	void VulkanContext::WindowResize()
	{
		device.WaitIdle();

		Memory::Delete(depthRt);

		Window* pMainWindow = OperatingSystem::GetMainWindow();
		swapchain.CreateOrRecreate(pMainWindow->GetWidth(), pMainWindow->GetHeight());
		for (UInt32 i = 0; i < colorRts.size(); ++i)
			colorRts[i] = swapchain.GetRenderTarget(i); // this will new a VulkanRenderTarget

		RenderTargetCreateDesc depthRtCI;
		depthRtCI.width = colorRts[0]->GetWidth();
		depthRtCI.height = colorRts[0]->GetHeight();
		depthRtCI.depth = colorRts[0]->GetDepth();
		depthRtCI.array = 1;
		depthRtCI.mipmap = 1;
		depthRtCI.format = EImageFormat::eUnorm_D24_uint_S8;
		depthRtCI.sample = ESampleCount::eSample_1;
		depthRtCI.type = EImageType::e2D;
		depthRtCI.usage = EImageUsage::efDepth_Stencil;

		depthRt = VulkanRenderTarget::Create(device, depthRtCI);

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
			.AddPoolElement(EBufferType::efUniform, 1)
			.SetMaxSets(1)
			.Build(device);

		if (!pDefaultDescriptorPool)
			SG_LOG_ERROR("Failed to create default descriptor pool!");

		pFences.resize(swapchain.imageCount);
		for (Size i = 0; i < swapchain.imageCount; ++i)
		{
			VulkanFence** ppFence = &pFences[i];
			*ppFence = VulkanFence::Create(device, true);
		}

		pRenderCompleteSemaphore = VulkanSemaphore::Create(device);
		pPresentCompleteSemaphore = VulkanSemaphore::Create(device);
	}

	void VulkanContext::DestroyDefaultResource()
	{
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