#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "Platform/OS.h"
#include "System/Logger.h"
#include "System/Input.h"
#include "Memory/Memory.h"

#include "Render/SwapChain.h"
#include "Render/ShaderComiler.h"
#include "Render/Camera/PointOrientedCamera.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

namespace SG
{

	VulkanRenderDevice::VulkanRenderDevice()
		:mCurrentFrameInCPU(0), mbBlockEvent(true)
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

		// create command buffer
		mpCommandBuffers.resize(mpContext->swapchain.imageCount);
		for (auto& pCmdBuf : mpCommandBuffers)
			mpContext->graphicCommandPool->AllocateCommandBuffer(pCmdBuf);

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

		CreateGeoBuffers(vertices, indices);
		CreateUBOBuffers();

		mpCameraUBOSetLayout = VulkanDescriptorSetLayout::Builder(mpContext->device)
			.AddBinding(EBufferType::efUniform, 0, 1)
			.Build();

		VulkanDescriptorDataBinder(*mpContext->pDefaultDescriptorPool, *mpCameraUBOSetLayout)
			.BindBuffer(0, mpCameraUBOBuffer)
			.Build(mpContext->cameraUBOSet);

		mpPipeline = Memory::New<VulkanPipeline>();
		mpPipeline->pRenderPass = VulkanRenderPass::Builder(mpContext->device)
			.BindColorRenderTarget(*mpContext->colorRts[0], EResourceBarrier::efUndefined, EResourceBarrier::efPresent)
			.BindDepthRenderTarget(*mpContext->depthRt, EResourceBarrier::efUndefined, EResourceBarrier::efDepth)
			.CombineAsSubpass()
			.Build();
		mpPipeline->pipelineCache = mpContext->device.CreatePipelineCache();
		mpPipeline->layout        = mpContext->device.CreatePipelineLayout(mpCameraUBOSetLayout->NativeHandle());
		BufferLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat3, "position" },
			{ EShaderDataType::eFloat3, "color" },
		};
		mpPipeline->pipeline = mpContext->device.CreatePipeline(mpPipeline->pipelineCache, mpPipeline->layout, mpPipeline->pRenderPass->NativeHandle(), mBasicShader, &vertexBufferLayout);

		mpFrameBuffers = Memory::New<VulkanFrameBuffer>();
		mpFrameBuffers->frameBuffers.resize(mpContext->swapchain.imageCount);
		for (UInt32 i = 0; i < mpFrameBuffers->frameBuffers.size(); ++i)
			mpFrameBuffers->frameBuffers[i] = mpContext->device.CreateFrameBuffer(mpPipeline->pRenderPass->NativeHandle(), mpContext->colorRts[i], mpContext->depthRt);

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

		SG_LOG_INFO("RenderDevice - Vulkan Init");

		RecordRenderCommands();
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpContext->graphicQueue.WaitIdle();

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

		Memory::Delete(mpCameraUBOSetLayout);

		mpContext->device.DestroyPipeline(mpPipeline->pipeline);
		mpContext->device.DestroyPipelineLayout(mpPipeline->layout);
		mpContext->device.DestroyPipelineCache(mpPipeline->pipelineCache);
		Memory::Delete(mpPipeline->pRenderPass);
		Memory::Delete(mpPipeline);

		DestroyUBOBuffers();
		DestroyGeoBuffers();

		Memory::Delete(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Shutdown");

		Memory::Delete(mpCamera);
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		static float totalTime = 0.0f;
		static float speed = 0.005f;
		TranslateToX(mCameraUBO.model, 0.5f * Sin(totalTime));

		if (mpCamera->IsViewDirty())
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

		mpContext->graphicQueue.SubmitCommands(&mpCommandBuffers[mCurrentFrameInCPU], mpRenderCompleteSemaphore, mpPresentCompleteSemaphore, mpBufferFences[mCurrentFrameInCPU]);
		mpContext->swapchain.Present(&mpContext->graphicQueue, mCurrentFrameInCPU, mpRenderCompleteSemaphore);

		mbBlockEvent = false;
	}

	bool VulkanRenderDevice::OnSystemMessage(ESystemMessage msg)
	{
		if (mbBlockEvent)
			return true;

		switch (msg)
		{
		case SG::ESystemMessage::eWindowMinimal: mbWindowMinimal = true;  break;
		case SG::ESystemMessage::eWindowResize:  mbWindowMinimal = false; WindowResize();
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
		const float  ASPECT = window->GetAspectRatio();
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
			frameBuffers[i] = mpContext->device.CreateFrameBuffer(mpPipeline->pRenderPass->NativeHandle(), colorRt, mpContext->depthRt);
		}

		for (auto& pCmdBuf : mpCommandBuffers)
		{
			mpContext->graphicCommandPool->FreeCommandBuffer(pCmdBuf);
			mpContext->graphicCommandPool->AllocateCommandBuffer(pCmdBuf);
		}

		RecordRenderCommands();
	}

	void VulkanRenderDevice::RecordRenderCommands()
	{
		auto* pipeline = static_cast<VulkanPipeline*>(mpPipeline);
		for (UInt32 i = 0; i < mpCommandBuffers.size(); ++i)
		{
			auto& pBuf = mpCommandBuffers[i];
			auto* pFb  = mpFrameBuffers->frameBuffers[i];
			auto* pColorRt = static_cast<VulkanRenderTarget*>(mpContext->colorRts[i]);

			pBuf.BeginRecord(true);

			pBuf.SetViewport((float)pColorRt->width, (float)pColorRt->height, 0.0f, 1.0f);
			pBuf.SetScissor({ 0, 0, (int)pColorRt->width, (int)pColorRt->height });

			ClearValue cv;
			cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			cv.depthStencil = { 1.0f, 0 };

			pBuf.BeginRenderPass(pipeline->pRenderPass->NativeHandle(), pFb, cv, pColorRt->width, pColorRt->height);
				pBuf.BindDescriptorSet(pipeline->layout, 0, mpContext->cameraUBOSet);
				pBuf.BindPipeline(pipeline->pipeline);

				UInt64 offset[1] = { 0 };
				pBuf.BindVertexBuffer(0, 1, *mpVertexBuffer, offset);
				pBuf.BindIndexBuffer(*mpIndexBuffer, 0);
				pBuf.DrawIndexed(6, 1, 0, 0, 1);
			pBuf.EndRenderPass();

			pBuf.EndRecord();
		}
	}

	bool VulkanRenderDevice::CreateGeoBuffers(float* vertices, UInt32* indices)
	{
		bool bSuccess = true;

		// vertex buffer
		BufferCreateDesc BufferCI = {};
		BufferCI.totalSizeInByte = sizeof(float) * 6 * 4;
		BufferCI.pData = vertices;
		BufferCI.type  = EBufferType::efVertex | EBufferType::efTransfer_Dst;
		mpVertexBuffer = VulkanBuffer::Create(mpContext->device, BufferCI, true);
		BufferCI.type = EBufferType::efTransfer_Src;
		VulkanBuffer* pVertexStagingBuffer = VulkanBuffer::Create(mpContext->device, BufferCI, false);
		pVertexStagingBuffer->UploadData(vertices);

		// index buffer
		BufferCI.totalSizeInByte = sizeof(UInt32) * 6;
		BufferCI.pData = indices;
		BufferCI.type = EBufferType::efIndex | EBufferType::efTransfer_Dst;
		mpIndexBuffer = VulkanBuffer::Create(mpContext->device, BufferCI, true);
		BufferCI.type = EBufferType::efTransfer_Src;
		VulkanBuffer* pIndexStagingBuffer = VulkanBuffer::Create(mpContext->device, BufferCI, false);
		pIndexStagingBuffer->UploadData(indices);

		/// begin copy buffers
		VulkanCommandBuffer pCmd;
		mpContext->transferCommandPool->AllocateCommandBuffer(pCmd);

		pCmd.BeginRecord();
		pCmd.CopyBuffer(*pVertexStagingBuffer, *mpVertexBuffer);
		pCmd.CopyBuffer(*pIndexStagingBuffer, *mpIndexBuffer);
		pCmd.EndRecord();

		auto* pFence = Memory::New<VulkanFence>();
		pFence->fence = mpContext->device.CreateFence();
		mpContext->transferQueue.SubmitCommands(&pCmd, nullptr, nullptr, pFence);
		mpContext->transferQueue.WaitIdle();

		Memory::Delete(pVertexStagingBuffer);
		Memory::Delete(pIndexStagingBuffer);
		mpContext->device.DestroyFence(pFence->fence);
		Memory::Delete(pFence);
		/// end copy buffers

		if (!mpVertexBuffer || !mpIndexBuffer)
			bSuccess &= false;
		return bSuccess;
	}

	void VulkanRenderDevice::DestroyGeoBuffers()
	{
		Memory::Delete(mpVertexBuffer);
		Memory::Delete(mpIndexBuffer);
	}

	bool VulkanRenderDevice::CreateUBOBuffers()
	{
		BufferCreateDesc BufferCI = {};
		BufferCI.totalSizeInByte = sizeof(UBO);
		BufferCI.pData = &mCameraUBO;
		BufferCI.type = EBufferType::efUniform;
		mpCameraUBOBuffer = VulkanBuffer::Create(mpContext->device, BufferCI, false);
		return mpCameraUBOBuffer != nullptr;
	}

	void VulkanRenderDevice::DestroyUBOBuffers()
	{
		Memory::Delete(mpCameraUBOBuffer);
	}

}