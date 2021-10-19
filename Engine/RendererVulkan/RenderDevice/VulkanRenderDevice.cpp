#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "Platform/IOperatingSystem.h"
#include "Platform/Window.h"
#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include "Render/SwapChain.h"
#include "Render/ShaderComiler.h"
#include "Render/Camera/PointOrientedCamera.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

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
		mpCamera = Memory::New<PointOrientedCamera>(Vector3f(0.0f, 0.0f, -2.0f));
		Vector3f modelPos      = { 0.0f, 0.0f, 0.0f };
		Vector3f modelScale    = { 0.25f, 0.25f, 1.0f };
		Vector3f modelRatation = { 0.0f, 0.0f, 0.0f };

		mCameraUBO.model = BuildTransformMatrix(modelPos, modelScale, modelRatation);
		mCameraUBO.view  = mpCamera->GetViewMatrix();
		mCameraUBO.proj  = mpCamera->GetProjMatrix();

		ShaderCompiler compiler;
		compiler.CompileGLSLShader("basic", mBasicShader);
		//SG_LOG_DEBUG("Num Stages: %d", mBasicShader.size());

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
		// data for a triangle
		float vertices[24] = {
			 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		};

		UInt32 indices[6] = {
			0, 1, 2,
			2, 3, 0
		};
		CreateAndUploadBuffers(vertices, indices);

		mpPipeline = Memory::New<VulkanPipeline>();
		mpPipeline->renderPass = mpDevice->CreateRenderPass(mpColorRts[0], mpDepthRt);
		mpPipeline->pipelineCache = mpDevice->CreatePipelineCache();
		mpPipeline->layout   = mpDevice->CreatePipelineLayout(mpCameraUBOBuffer);
		BufferLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat3, "position" },
			{ EShaderDataType::eFloat3, "color" },
		};
		mpPipeline->pipeline = mpDevice->CreatePipeline(mpPipeline->pipelineCache, mpPipeline->layout, mpPipeline->renderPass, mBasicShader, &vertexBufferLayout);

		mpCameraUBOBuffer->descriptorSet = mpDevice->AllocateDescriptorSet(mpCameraUBOBuffer->descriptorSetLayout);
		mpCameraUBOBuffer->UpdateDescriptor();
		mpCameraUBOBuffer->UploadData(&mCameraUBO);

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

		for (UInt32 i = 0; i < mpFrameBuffers->frameBuffers.size(); ++i)
			mpDevice->DestroyFrameBuffer(mpFrameBuffers->frameBuffers[i]);
		Memory::Delete(mpFrameBuffers);

		//mpDevice->FreeDescriptorSet(mpCameraUBOBuffer->descriptorSet);

		mpDevice->DestroyPipeline(mpPipeline->pipeline);
		mpDevice->DestroyPipelineLayout(mpPipeline->layout);
		mpDevice->DestroyPipelineCache(mpPipeline->pipelineCache);
		mpDevice->DestroyRenderPass(mpPipeline->renderPass);
		Memory::Delete(mpPipeline);

		DestroyDepthRT();
		DestroyBuffers();
		mpDevice->DestroyRenderContext(mpRenderContext);

		mpSwapchain->Destroy();
		Memory::Delete(mpSwapchain);
		Memory::Delete(mpDevice);

		Memory::Delete(mpInstance);
		SG_LOG_DEBUG("RenderDevice - Vulkan Shutdown");

		Memory::Delete(mpCamera);
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		static float totalTime = 0.0f;
		static float speed = 0.005f;
		Vector3f pos = { 0.5f * Sin(totalTime), 0.0f, 0.0f };
		TranslateTo(mCameraUBO.model, pos);
		//Rotate(mCameraUBO.model, SG_ENGINE_UP_VEC(), deltaTime * 0.5f);

		mpCameraUBOBuffer->UploadData(&mCameraUBO);
		totalTime += deltaTime * speed;
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

			mpRenderContext->CmdBindDescriptorSets(pBuf, pipeline->layout, mpCameraUBOBuffer->descriptorSet);
			mpRenderContext->CmdBindPipeline(pBuf, pipeline->pipeline);
			VkDeviceSize offset[1] = { 0 };
			mpRenderContext->CmdBindVertexBuffer(pBuf, 0, 1, &mpVertexBuffer->buffer, offset);
			mpRenderContext->CmdBindIndexBuffer(pBuf, mpIndexBuffer->buffer, 0);

			mpRenderContext->CmdDrawIndexed(pBuf, 6, 1, 0, 0, 1);

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

	bool VulkanRenderDevice::CreateAndUploadBuffers(float* vertices, UInt32* indices)
	{
		bool bSuccess = true;

		BufferCreateDesc BufferCI = {};
		BufferCI.totalSizeInByte = sizeof(float) * 6 *4;
		BufferCI.pData = vertices;
		BufferCI.type  = EBufferType::efVertex | EBufferType::efTransfer_Dst;
		mpVertexBuffer = mpDevice->CreateBuffer(BufferCI);

		BufferCI.totalSizeInByte = sizeof(UInt32) * 6;
		BufferCI.pData = indices;
		BufferCI.type = EBufferType::efIndex | EBufferType::efTransfer_Dst;
		mpIndexBuffer  = mpDevice->CreateBuffer(BufferCI);

		BufferCI.totalSizeInByte = sizeof(UBO);
		BufferCI.pData = &mCameraUBO;
		BufferCI.type = EBufferType::efUniform;
		mpCameraUBOBuffer = mpDevice->CreateBuffer(BufferCI);

		if (!mpVertexBuffer || !mpIndexBuffer)
			bSuccess &= false;
		return bSuccess;
	}

	void VulkanRenderDevice::DestroyBuffers()
	{
		mpDevice->DestroyBuffer(mpCameraUBOBuffer);
		mpDevice->DestroyBuffer(mpVertexBuffer);
		mpDevice->DestroyBuffer(mpIndexBuffer);
		Memory::Delete(mpCameraUBOBuffer);
		Memory::Delete(mpVertexBuffer);
		Memory::Delete(mpIndexBuffer);
	}

}