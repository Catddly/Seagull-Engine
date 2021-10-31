#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "Platform/OS.h"
#include "System/Logger.h"
#include "System/Input.h"
#include "Memory/Memory.h"

#include "Render/SwapChain.h"
#include "Render/ShaderComiler.h"
#include "Render/Camera/PointOrientedCamera.h"

#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanRenderContext.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

namespace SG
{

	VulkanRenderDevice::VulkanRenderDevice()
		:mCurrentFrameInCPU(0)
	{
		SSystem()->RegisterSystemMessageListener(this);
		Input::RegisterListener(this);
	}

	VulkanRenderDevice::~VulkanRenderDevice()
	{
		SSystem()->RemoveSystemMessageListener(this);
		Input::RemoveListener(this);
	}

	void VulkanRenderDevice::OnInit()
	{
		/// begin outer resource preparation
		auto* window = OperatingSystem::GetMainWindow();
		const float ASPECT = window->GetAspectRatio();

		mpCamera = Memory::New<PointOrientedCamera>(Vector3f(0.0f, 0.0f, -3.0f));
		mpCamera->SetPerspective(45.0f, ASPECT);
		Vector3f modelPos      = { 0.0f, 0.0f, -1.0f };
		Vector3f modelScale    = { 0.25f, 0.25f, 1.0f };
		Vector3f modelRatation = { 0.0f, 0.0f, 0.0f };

		mCameraUBO.model = BuildTransformMatrix(modelPos, modelScale, modelRatation);
		mCameraUBO.view  = mpCamera->GetViewMatrix();
		mCameraUBO.proj  = mpCamera->GetProjMatrix();

		ShaderCompiler compiler;
		compiler.CompileGLSLShader("basic", mBasicShader);
		/// end  outer resource preparation

		mpContext = Memory::New<VulkanContext>();
		mpQueue = mpContext->device.GetQueue(EQueueType::eGraphic);

		// create command buffer
		mpRenderContext = mpContext->device.CreateRenderContext(mpContext->swapchain.imageCount, mpContext->graphicCommandPool);
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
		CreateBuffers(vertices, indices);

		mpPipeline = Memory::New<VulkanPipeline>();
		mpPipeline->renderPass = mpContext->device.CreateRenderPass(mpContext->colorRts[0], mpContext->depthRt);
		mpPipeline->pipelineCache = mpContext->device.CreatePipelineCache();
		mpPipeline->layout   = mpContext->device.CreatePipelineLayout(mpCameraUBOBuffer);
		BufferLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat3, "position" },
			{ EShaderDataType::eFloat3, "color" },
		};
		mpPipeline->pipeline = mpContext->device.CreatePipeline(mpPipeline->pipelineCache, mpPipeline->layout, mpPipeline->renderPass, mBasicShader, &vertexBufferLayout);

		//mpCameraUBOBuffer->descriptorSet = mpContext->device.AllocateDescriptorSet(mpCameraUBOBuffer->descriptorSetLayout, mpContext->defaultDescriptorPool);
		//mpCameraUBOBuffer->UpdateDescriptor(mpContext->device.logicalDevice);

		mpFrameBuffers = Memory::New<VulkanFrameBuffer>();
		mpFrameBuffers->frameBuffers.resize(mpContext->swapchain.imageCount);
		for (UInt32 i = 0; i < mpFrameBuffers->frameBuffers.size(); ++i)
			mpFrameBuffers->frameBuffers[i] = mpContext->device.CreateFrameBuffer(mpPipeline->renderPass, mpContext->colorRts[i], mpContext->depthRt);

		mpRenderCompleteSemaphore = Memory::New<VulkanSemaphore>();
		mpRenderCompleteSemaphore->semaphore = mpContext->device.CreateSemaphore();
		mpPresentCompleteSemaphore = Memory::New<VulkanSemaphore>();
		mpPresentCompleteSemaphore->semaphore = mpContext->device.CreateSemaphore();

		mpBufferFences.resize(mpContext->swapchain.imageCount);
		for (UInt32 i = 0; i < mpContext->swapchain.imageCount; ++i)
		{
			mpBufferFences[i] = Memory::New<VulkanFence>();
			mpBufferFences[i]->fence = mpContext->device.CreateFence(true);
		}

		SG_LOG_DEBUG("RenderDevice - Vulkan Init");

		RecordRenderCommands();
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpQueue->WaitIdle();
		Memory::Delete(mpQueue);

		mpContext->device.DestroySemaphore(mpRenderCompleteSemaphore->semaphore);
		mpContext->device.DestroySemaphore(mpPresentCompleteSemaphore->semaphore);
		Memory::Delete(mpRenderCompleteSemaphore);
		Memory::Delete(mpPresentCompleteSemaphore);
		for (UInt32 i = 0; i < mpContext->swapchain.imageCount; ++i)
		{
			mpContext->device.DestroyFence(mpBufferFences[i]->fence);
			Memory::Delete(mpBufferFences[i]);
		}

		for (UInt32 i = 0; i < mpFrameBuffers->frameBuffers.size(); ++i)
			mpContext->device.DestroyFrameBuffer(mpFrameBuffers->frameBuffers[i]);
		Memory::Delete(mpFrameBuffers);

		mpContext->device.DestroyPipeline(mpPipeline->pipeline);
		mpContext->device.DestroyPipelineLayout(mpPipeline->layout);
		mpContext->device.DestroyPipelineCache(mpPipeline->pipelineCache);
		mpContext->device.DestroyRenderPass(mpPipeline->renderPass);
		Memory::Delete(mpPipeline);

		DestroyBuffers();
		mpContext->device.DestroyRenderContext(mpRenderContext);

		Memory::Delete(mpContext);
		SG_LOG_DEBUG("RenderDevice - Vulkan Shutdown");

		Memory::Delete(mpCamera);
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		static float totalTime = 0.0f;
		static float speed = 0.005f;
		TranslateToX(mCameraUBO.model, 0.5f * Sin(totalTime));

		mCameraUBO.view = mpCamera->GetViewMatrix();
		mpCameraUBOBuffer->UploadData(&mCameraUBO);

		totalTime += deltaTime * speed;
	}

	void VulkanRenderDevice::OnDraw()
	{
		if (mbWindowMinimal)
			return;

		mpContext->swapchain.AcquireNextImage(mpPresentCompleteSemaphore, mCurrentFrameInCPU);
		mpContext->device.ResetFence(mpBufferFences[mCurrentFrameInCPU]->fence);

		mpQueue->SubmitCommands(mpRenderContext, mCurrentFrameInCPU, mpRenderCompleteSemaphore, mpPresentCompleteSemaphore, mpBufferFences[mCurrentFrameInCPU]);
		mpContext->swapchain.Present(mpQueue, mCurrentFrameInCPU, mpRenderCompleteSemaphore);

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

	bool VulkanRenderDevice::OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState)
	{
		if (keycode == KeyCode_T && keyState == EKeyState::ePressed)
		{
			mbUseOrtho = !mbUseOrtho;
			auto* window = OperatingSystem::GetMainWindow();
			const float ASPECT = window->GetAspectRatio();
			if (mbUseOrtho)
				mpCamera->SetOrthographic(-ASPECT, ASPECT, 1, -1, -1, 1);
			else
				mpCamera->SetPerspective(45.0f, ASPECT);
			mCameraUBO.proj = mpCamera->GetProjMatrix();
			mpCameraUBOBuffer->UploadData(&mCameraUBO);
		}
		return true;
	}

	void VulkanRenderDevice::WindowResize()
	{
		auto* window = OperatingSystem::GetMainWindow();
		const UInt32 WIDTH = window->GetWidth();
		const UInt32 HEIGHT = window->GetHeight();
		const float  ASPECT = (float)WIDTH / HEIGHT;
		if (mbUseOrtho)
			mpCamera->SetOrthographic(-ASPECT, ASPECT, 1, -1, -1, 1);
		else
			mpCamera->SetPerspective(45.0f, ASPECT);
		mCameraUBO.proj = mpCamera->GetProjMatrix();

		mpContext->WindowResize();

		auto& frameBuffers = mpFrameBuffers->frameBuffers;
		for (UInt32 i = 0; i < frameBuffers.size(); ++i)
		{
			auto* colorRt = mpContext->colorRts[i];
			mpContext->device.DestroyFrameBuffer(frameBuffers[i]);
			frameBuffers[i] = mpContext->device.CreateFrameBuffer(mpPipeline->renderPass, colorRt, mpContext->depthRt);
		}

		mpContext->device.FreeCommandBuffers(mpRenderContext);
		mpContext->device.AllocateCommandBuffers(mpRenderContext);

		RecordRenderCommands();
	}

	void VulkanRenderDevice::RecordRenderCommands()
	{
		auto* pipeline = static_cast<VulkanPipeline*>(mpPipeline);
		for (UInt32 i = 0; i < mpRenderContext->commandBuffers.size(); ++i)
		{
			auto* pBuf = mpRenderContext->commandBuffers[i];
			auto* pFb  = mpFrameBuffers->frameBuffers[i];
			auto* pColorRt = static_cast<VulkanRenderTarget*>(mpContext->colorRts[i]);
			mpRenderContext->CmdBeginCommandBuf(pBuf, true);
			ClearValue cv;
			cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			cv.depthStencil = { 1.0f, 0 };
			mpRenderContext->CmdBeginRenderPass(pBuf, pipeline->renderPass, pFb, cv, pColorRt->width, pColorRt->height);

			mpRenderContext->CmdSetViewport(pBuf, (float)pColorRt->width, (float)pColorRt->height, 0.0f, 1.0f);
			mpRenderContext->CmdSetScissor(pBuf, { 0, 0, (int)pColorRt->width, (int)pColorRt->height });

			//mpRenderContext->CmdBindDescriptorSets(pBuf, pipeline->layout, mpCameraUBOBuffer->descriptorSet);
			mpRenderContext->CmdBindPipeline(pBuf, pipeline->pipeline);
			VkDeviceSize offset[1] = { 0 };
			mpRenderContext->CmdBindVertexBuffer(pBuf, 0, 1, &mpVertexBuffer->NativeHandle(), offset);
			mpRenderContext->CmdBindIndexBuffer(pBuf, mpIndexBuffer->NativeHandle(), 0);

			mpRenderContext->CmdDrawIndexed(pBuf, 6, 1, 0, 0, 1);

			mpRenderContext->CmdEndRenderPass(pBuf);
			mpRenderContext->CmdEndCommandBuf(pBuf);
		}

		SG_LOG_DEBUG("RenderDevice - Render Command Recorded");
	}

	bool VulkanRenderDevice::CreateBuffers(float* vertices, UInt32* indices)
	{
		bool bSuccess = true;

		BufferCreateDesc BufferCI = {};
		BufferCI.totalSizeInByte = sizeof(float) * 6 * 4;
		BufferCI.pData = vertices;
		BufferCI.type  = EBufferType::efVertex | EBufferType::efTransfer_Dst;
		mpVertexBuffer = VulkanBuffer::Create(mpContext->device, BufferCI, true);

		BufferCI.totalSizeInByte = sizeof(UInt32) * 6;
		BufferCI.pData = indices;
		BufferCI.type = EBufferType::efIndex | EBufferType::efTransfer_Dst;
		mpIndexBuffer  = VulkanBuffer::Create(mpContext->device, BufferCI, true);

		BufferCI.totalSizeInByte = sizeof(UBO);
		BufferCI.pData = &mCameraUBO;
		BufferCI.type = EBufferType::efUniform;
		mpCameraUBOBuffer = VulkanBuffer::Create(mpContext->device, BufferCI, false);

		if (!mpVertexBuffer || !mpIndexBuffer)
			bSuccess &= false;
		return bSuccess;
	}

	void VulkanRenderDevice::DestroyBuffers()
	{
		Memory::Delete(mpCameraUBOBuffer);
		Memory::Delete(mpVertexBuffer);
		Memory::Delete(mpIndexBuffer);
	}

}