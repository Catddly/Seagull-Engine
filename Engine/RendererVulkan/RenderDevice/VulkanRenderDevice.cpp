#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "Platform/IOperatingSystem.h"
#include "Platform/Window.h"
#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include "Render/SwapChain.h"
#include "Render/ShaderComiler.h"

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
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("basic", mBasicShader);
		SG_LOG_DEBUG("Num Stages: %d", mBasicShader.size());

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
		mpRenderContext = mpDevice->CreateRenderContext(mpSwapchain->imageCount, EQueueType::eGraphic);

		CreateDepthRT();

		mpPipeline = Memory::New<VulkanPipeline>();
		mpPipeline->renderPass = mpDevice->CreateRenderPass(mpColorRts[0], mpDepthRt);
		mpPipeline->pipelineCache = mpDevice->CreatePipelineCache();;
		mpPipeline->pipeline = mpDevice->CreatePipeline(mpPipeline->pipelineCache, mpPipeline->renderPass, mBasicShader);

		mpFrameBuffers = Memory::New<VulkanFrameBuffer>();
		mpFrameBuffers->frameBuffers.resize(mpSwapchain->imageCount);
		for (UInt32 i = 0; i < mpFrameBuffers->frameBuffers.size(); ++i)
			mpFrameBuffers->frameBuffers[i] = mpDevice->CreateFrameBuffer(mpPipeline->renderPass, mpColorRts[i], mpDepthRt);

		mpRenderCompleteSemaphore = Memory::New<VulkanSemaphore>();
		mpRenderCompleteSemaphore->semaphore = mpDevice->CreateSemaphore();
		mpPresentCompleteSemaphore = Memory::New<VulkanSemaphore>();
		mpPresentCompleteSemaphore->semaphore = mpDevice->CreateSemaphore();

		mpBufferFences.resize(mpSwapchain->imageCount);
		for (UInt32 i = 0; i < mpSwapchain->imageCount; ++i)
		{
			mpBufferFences[i] = Memory::New<VulkanFence>();
			mpBufferFences[i]->fence = mpDevice->CreateFence(true);
		}

		SG_LOG_DEBUG("RenderDevice - Vulkan Init");

		CreateAndUploadBuffers();

		RecordRenderCommands();
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpQueue->WaitIdle();

		DestroyBuffers();

		mpDevice->DestroySemaphore(mpRenderCompleteSemaphore->semaphore);
		mpDevice->DestroySemaphore(mpPresentCompleteSemaphore->semaphore);
		Memory::Delete(mpRenderCompleteSemaphore);
		Memory::Delete(mpPresentCompleteSemaphore);
		for (UInt32 i = 0; i < mpSwapchain->imageCount; ++i)
		{
			mpDevice->DestroyFence(mpBufferFences[i]->fence);
			Memory::Delete(mpBufferFences[i]);
		}

		for (UInt32 i = 0; i < mpFrameBuffers->frameBuffers.size(); ++i)
			mpDevice->DestroyFrameBuffer(mpFrameBuffers->frameBuffers[i]);
		Memory::Delete(mpFrameBuffers);

		mpDevice->DestroyPipeline(mpPipeline->pipeline);
		mpDevice->DestroyPipelineCache(mpPipeline->pipelineCache);
		mpDevice->DestroyRenderPass(mpPipeline->renderPass);
		Memory::Delete(mpPipeline);

		DestroyDepthRT();
		mpDevice->DestroyRenderContext(mpRenderContext);

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

		mbBlockEvent = false;
	}

	bool VulkanRenderDevice::OnSystemMessage(ESystemMessage msg)
	{
		if (mbBlockEvent)
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

		auto& frameBuffers = mpFrameBuffers->frameBuffers;
		auto* pipeline = static_cast<VulkanPipeline*>(mpPipeline);
		for (UInt32 i = 0; i < frameBuffers.size(); ++i)
		{
			auto* colorRt = static_cast<VulkanRenderTarget*>(mpColorRts[i]);
			mpDevice->DestroyFrameBuffer(frameBuffers[i]);
			frameBuffers[i] = mpDevice->CreateFrameBuffer(pipeline->renderPass, colorRt, static_cast<VulkanRenderTarget*>(mpDepthRt));
		}

		mpDevice->FreeCommandBuffers(mpRenderContext);
		mpDevice->AllocateCommandBuffers(mpRenderContext);

		RecordRenderCommands();
	}

	void VulkanRenderDevice::RecordRenderCommands()
	{
		auto* pipeline = static_cast<VulkanPipeline*>(mpPipeline);
		for (UInt32 i = 0; i < mpRenderContext->commandBuffers.size(); ++i)
		{
			auto* pBuf = mpRenderContext->commandBuffers[i];
			auto* pFb  = mpFrameBuffers->frameBuffers[i];
			auto* pColorRt = static_cast<VulkanRenderTarget*>(mpColorRts[i]);
			mpRenderContext->CmdBeginCommandBuf(pBuf, true);
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

	bool VulkanRenderDevice::CreateAndUploadBuffers()
	{
		bool bSuccess = true;

		// datas for a triangle
		float vertices[3][3] = {
			{  1.0f,  1.0f, 0.0f },
			{ -1.0f,  1.0f, 0.0f },
			{  0.0f, -1.0f, 0.0f },
		};

		UInt32 indices[3] = {
			0, 1, 2
		};

		BufferCreateDesc vertexBufferCI = {};
		vertexBufferCI.sizeInByte = sizeof(float) * 3 * 3;
		vertexBufferCI.pData = vertices;
		vertexBufferCI.type  = EBufferType::efVertex | EBufferType::efTransfer_Dst;

		BufferCreateDesc indexBufferCI = {};
		indexBufferCI.sizeInByte = sizeof(UInt32) * 3;
		indexBufferCI.pData = indices;
		indexBufferCI.type = EBufferType::efIndex | EBufferType::efTransfer_Dst;

		mpVertexBuffer = mpDevice->CreateBuffer(vertexBufferCI);
		mpIndexBuffer  = mpDevice->CreateBuffer(indexBufferCI);

		//bSuccess &= mpVertexBuffer->Upload(mpDevice->logicalDevice, vertices);
		//bSuccess &= mpIndexBuffer->Upload(mpDevice->logicalDevice, indices);

		if (!mpVertexBuffer || !mpIndexBuffer)
			bSuccess &= false;
		return bSuccess;
	}

	void VulkanRenderDevice::DestroyBuffers()
	{
		mpDevice->DestroyBuffer(mpVertexBuffer);
		mpDevice->DestroyBuffer(mpIndexBuffer);
		Memory::Delete(mpVertexBuffer);
		Memory::Delete(mpIndexBuffer);
	}

}