#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "Platform/IOperatingSystem.h"
#include "Platform/Window.h"
#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include "Render/SwapChain.h"

#include "RendererVulkan/Backend/VulkanInstance.h"
#include "RendererVulkan/Backend/VulkanRenderContext.h"
#include "RendererVulkan/Backend/VulkanDevice.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"

namespace SG
{

	VulkanRenderDevice::VulkanRenderDevice()
		:mCurrentFrameInCPU(0)
	{
		SSystem()->RegisterSystemMessageListener(this);
	}

	VulkanRenderDevice::~VulkanRenderDevice()
	{
		SSystem()->RemoveSystemMessageListener(this);
	}

	void VulkanRenderDevice::OnInit()
	{
		mpInstance = Memory::New<VulkanInstance>();
		mpSwapchain = Memory::New<VulkanSwapchain>(mpInstance->instance);
		mpSwapchain->CreateSurface();

		if (!SelectPhysicalDeviceAndCreateDevice())
		{
			SG_LOG_ERROR("No suittable GPU detected!");
			SG_ASSERT(false);
		}
		mpDevice->CreateLogicalDevice(nullptr);
		mpQueue = mpDevice->GetQueue(EQueueType::eGraphic);

		mpSwapchain->BindDevice(mpDevice->physicalDevice, mpDevice->logicalDevice);
		mpSwapchain->CheckSurfacePresentable(mpQueue);

		Window* mainWindow = SSystem()->GetOS()->GetMainWindow();
		Rect rect = mainWindow->GetCurrRect();
		mpSwapchain->CreateOrRecreate(GetRectWidth(rect), GetRectHeight(rect));
		mpColorRts.resize(mpSwapchain->imageCount);
		for (UInt32 i = 0; i < mpColorRts.size(); ++i)
			mpColorRts[i] = mpSwapchain->GetRenderTarget(i);

		// create command buffer
		mpRenderContext = Memory::New<VulkanRenderContext>(mpSwapchain->imageCount);
		mpDevice->AllocateCommandBuffers(mpRenderContext->commandBuffers);

		CreateDepthRT();

		ShaderStages basicShader;
		mShaderCompiler.LoadSPIRVShader("basic", basicShader);

		mpPipeline = Memory::New<VulkanPipeline>();
		mpPipeline->renderPass = mpDevice->CreateRenderPass(mpColorRts[0], mpDepthRt);
		mpPipeline->pipelineCache = mpDevice->CreatePipelineCache();;
		mpPipeline->pipeline = mpDevice->CreatePipeline(mpPipeline->pipelineCache, mpPipeline->renderPass, basicShader);;

		for (UInt32 i = 0; i < mpRenderContext->frameBuffers.size(); ++i)
			mpRenderContext->frameBuffers[i] = mpDevice->CreateFrameBuffer(mpPipeline->renderPass, mpColorRts[i], mpDepthRt);

		mpRenderCompleteSemaphore = Memory::New<VulkanSemaphore>();
		mpRenderCompleteSemaphore->semaphore = mpDevice->CreateSemaphore();
		mpPresentCompleteSemaphore = Memory::New<VulkanSemaphore>();
		mpPresentCompleteSemaphore->semaphore = mpDevice->CreateSemaphore();

		mpBufferFences.resize(mpSwapchain->imageCount);
		for (UInt32 i = 0; i < mpSwapchain->imageCount; ++i)
		{
			mpBufferFences[i] = Memory::New<VulkanFence>();
			mpBufferFences[i]->fence = mpDevice->CreateFence();
		}

		SG_LOG_DEBUG("RenderDevice - Vulkan Init");

		RecordRenderCommands();
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpQueue->WaitIdle();

		mpDevice->DestroySemaphore(mpRenderCompleteSemaphore->semaphore);
		mpDevice->DestroySemaphore(mpPresentCompleteSemaphore->semaphore);
		Memory::Delete(mpRenderCompleteSemaphore);
		Memory::Delete(mpPresentCompleteSemaphore);
		for (UInt32 i = 0; i < mpSwapchain->imageCount; ++i)
		{
			mpDevice->DestroyFence(mpBufferFences[i]->fence);
			Memory::Delete(mpBufferFences[i]);
		}

		for (UInt32 i = 0; i < mpRenderContext->frameBuffers.size(); ++i)
			mpDevice->DestroyFrameBuffer(mpRenderContext->frameBuffers[i]);

		mpDevice->DestroyPipeline(mpPipeline->pipeline);
		mpDevice->DestroyPipelineCache(mpPipeline->pipelineCache);
		mpDevice->DestroyRenderPass(mpPipeline->renderPass);
		Memory::Delete(mpPipeline);

		DestroyDepthRT();
		mpDevice->FreeCommandBuffers(mpRenderContext->commandBuffers);
		Memory::Delete(mpRenderContext);

		mpSwapchain->Destroy();
		Memory::Delete(mpSwapchain);
		Memory::Delete(mpDevice);

		Memory::Delete(mpInstance);
		SG_LOG_DEBUG("RenderDevice - Vulkan Shutdown");
	}

	void VulkanRenderDevice::OnUpdate()
	{

	}

	void VulkanRenderDevice::OnDraw()
	{
		if (mbWindowMinimal)
			return;

		mpSwapchain->AcquireNextImage(mpPresentCompleteSemaphore, mCurrentFrameInCPU);

		auto* fence = static_cast<VulkanFence*>(mpBufferFences[mCurrentFrameInCPU]);
		mpDevice->ResetFence(fence->fence);

		mpQueue->SubmitCommands(mpRenderContext, mCurrentFrameInCPU, mpRenderCompleteSemaphore, mpPresentCompleteSemaphore, mpBufferFences[mCurrentFrameInCPU]);

		mpSwapchain->Present(mpQueue, mCurrentFrameInCPU, mpRenderCompleteSemaphore);

		mbPresentOnce = true;
	}

	bool VulkanRenderDevice::OnSystemMessage(ESystemMessage msg)
	{
		if (!mbPresentOnce)
			return true;

		switch (msg)
		{
		case SG::ESystemMessage::eWindowMinimal: SG_LOG_DEBUG("Window Minimized!"); mbWindowMinimal = true;  break;
		case SG::ESystemMessage::eWindowResize:  SG_LOG_DEBUG("Window Resizing!");  mbWindowMinimal = false; WindowResize();
		}
		return true;
	}

	bool VulkanRenderDevice::SelectPhysicalDeviceAndCreateDevice()
	{
		UInt32 gpuCount;
		vkEnumeratePhysicalDevices(mpInstance->instance, &gpuCount, nullptr);
		vector<VkPhysicalDevice> devices(gpuCount);
		vkEnumeratePhysicalDevices(mpInstance->instance, &gpuCount, devices.data());

		// TODO: add more conditions to choose the best device(adapter)
		for (auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures   deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
			//SG_LOG_DEBUG("VkAdapter Name: %s", deviceProperties.deviceName);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				mpDevice = Memory::New<VulkanDevice>(device);
				return true;
			}

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			{
				mpDevice = Memory::New<VulkanDevice>(device);
				SG_LOG_WARN("Integrated GPU detected!");
				return true;
			}
		}

		return false;
	}

	void VulkanRenderDevice::WindowResize()
	{
		mpDevice->WaitIdle();

		auto* window = SSystem()->GetOS()->GetMainWindow();
		Rect& rect = window->GetCurrRect();
		mpSwapchain->CreateOrRecreate(GetRectWidth(rect), GetRectHeight(rect));
		for (UInt32 i = 0; i < mpColorRts.size(); ++i)
			mpColorRts[i] = mpSwapchain->GetRenderTarget(i);

		DestroyDepthRT();
		CreateDepthRT();

		auto& frameBuffers = mpRenderContext->frameBuffers;
		auto* pipeline = static_cast<VulkanPipeline*>(mpPipeline);
		for (UInt32 i = 0; i < frameBuffers.size(); ++i)
		{
			auto* colorRt = static_cast<VulkanRenderTarget*>(mpColorRts[i]);
			mpDevice->DestroyFrameBuffer(frameBuffers[i]);
			frameBuffers[i] = mpDevice->CreateFrameBuffer(pipeline->renderPass, colorRt, static_cast<VulkanRenderTarget*>(mpDepthRt));
		}

		mpDevice->FreeCommandBuffers(mpRenderContext->commandBuffers);
		mpDevice->AllocateCommandBuffers(mpRenderContext->commandBuffers);
		RecordRenderCommands();
	}

	void VulkanRenderDevice::RecordRenderCommands()
	{
		auto* pipeline = static_cast<VulkanPipeline*>(mpPipeline);
		for (UInt32 i = 0; i < mpRenderContext->commandBuffers.size(); ++i)
		{
			auto* pBuf = mpRenderContext->commandBuffers[i];
			auto* pFb = mpRenderContext->frameBuffers[i];
			auto* pColorRt = static_cast<VulkanRenderTarget*>(mpColorRts[i]);
			mpRenderContext->CmdBeginCommandBuf(pBuf);
			ClearValue cv;
			cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			cv.depthStencil = { 1.0f, 0 };
			mpRenderContext->CmdBeginRenderPass(pBuf, pipeline->renderPass, pFb, cv, pColorRt->width, pColorRt->height);

			mpRenderContext->CmdSetViewport(pBuf, (float)pColorRt->width, (float)pColorRt->height, 0.0f, 1.0f);
			mpRenderContext->CmdSetScissor(pBuf, { 0, 0, (int)pColorRt->width, (int)pColorRt->height });

			mpRenderContext->CmdBindPipeline(pBuf, pipeline->pipeline);
			mpRenderContext->CmdDraw(pBuf, 3, 1, 0, 0);

			mpRenderContext->CmdEndRenderPass(pBuf);
			mpRenderContext->CmdEndCommandBuf(pBuf);
		}

		SG_LOG_DEBUG("RenderDevice - Render Command Recorded");
	}

	bool VulkanRenderDevice::CreateDepthRT()
	{
		RenderTargetCreateDesc depthRtCI;
		depthRtCI.width = mpColorRts[0]->GetWidth();
		depthRtCI.height = mpColorRts[0]->GetHeight();
		depthRtCI.depth = mpColorRts[0]->GetDepth();
		depthRtCI.array = 1;
		depthRtCI.mipmap = 1;
		depthRtCI.format = EImageFormat::eUnorm_D24_uint_S8;
		depthRtCI.sample = ESampleCount::eSample_1;
		depthRtCI.type = EImageType::e2D;
		depthRtCI.usage = ERenderTargetUsage::efDepth_Stencil;
		mpDepthRt = mpDevice->CreateRenderTarget(depthRtCI);
		if (!mpDepthRt)
			return false;
		return true;
	}

	void VulkanRenderDevice::DestroyDepthRT()
	{
		mpDevice->DestroyRenderTarget(static_cast<VulkanRenderTarget*>(mpDepthRt));
		Memory::Delete(mpDepthRt);
	}

}