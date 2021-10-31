#include "StdAfx.h"
#include "VulkanContext.h"

#include "Platform/OS.h"
#include "Memory/Memory.h"

namespace SG
{

	VulkanContext::VulkanContext()
		:instance(), swapchain(instance, device), device(instance.physicalDevice)
	{
		CreateDefaultResource();

		Window* pMainWindow = OperatingSystem::GetMainWindow();
		swapchain.CreateOrRecreate(pMainWindow->GetWidth(), pMainWindow->GetHeight());

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
		depthRtCI.usage = ERenderTargetUsage::efDepth_Stencil;

		depthRt = VulkanRenderTarget::Create(device, depthRtCI);
	}

	VulkanContext::~VulkanContext()
	{
		DestroyDefaultResource();

		swapchain.CleanUp();
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
		depthRtCI.usage = ERenderTargetUsage::efDepth_Stencil;

		depthRt = VulkanRenderTarget::Create(device, depthRtCI);
	}

	void VulkanContext::CreateDefaultResource()
	{
		// create a default command pool to allocate commands to graphic queue.
		graphicCommandPool = device.CreateCommandPool(device.queueFamilyIndices.graphics);
		if (graphicCommandPool == VK_NULL_HANDLE)
			SG_LOG_ERROR("Failed to create default graphic command pool!");

		// create a default command pool to allocate commands to transfer queue.
		if (device.queueFamilyIndices.transfer == device.queueFamilyIndices.graphics)
			transferCommandPool = graphicCommandPool;
		else
		{
			transferCommandPool = device.CreateCommandPool(device.queueFamilyIndices.transfer);
			if (transferCommandPool == VK_NULL_HANDLE)
				SG_LOG_ERROR("Failed to create default transfer command pool!");
		}

		// create a default command pool to allocate commands to compute queue.
		if (device.queueFamilyIndices.compute == device.queueFamilyIndices.graphics)
			computeCommandPool = graphicCommandPool;
		else
		{
			computeCommandPool = device.CreateCommandPool(device.queueFamilyIndices.compute);
			if (computeCommandPool == VK_NULL_HANDLE)
				SG_LOG_ERROR("Failed to create default compute command pool!");
		}

		defaultDescriptorPool = device.CreateDescriptorPool();
		if (defaultDescriptorPool == VK_NULL_HANDLE)
			SG_LOG_ERROR("Failed to create default descriptor pool!");
	}

	void VulkanContext::DestroyDefaultResource()
	{
		device.DestroyDescriptorPool(defaultDescriptorPool);
		if (computeCommandPool != VK_NULL_HANDLE && device.queueFamilyIndices.graphics != device.queueFamilyIndices.compute)
			device.DestroyCommandPool(computeCommandPool);
		if (transferCommandPool != VK_NULL_HANDLE && device.queueFamilyIndices.graphics != device.queueFamilyIndices.transfer)
			device.DestroyCommandPool(transferCommandPool);
		if (graphicCommandPool != VK_NULL_HANDLE)
			device.DestroyCommandPool(graphicCommandPool);
	}

}