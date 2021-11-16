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

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

namespace SG
{

	VulkanRenderDevice::VulkanRenderDevice()
		:mCurrentFrameInCPU(0), mbBlockEvent(true), mRenderGraph("Default")
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

		mpCamera = Memory::New<PointOrientedCamera>(Vector3f(0.0f, 0.0f, -4.0f));
		mpCamera->SetPerspective(45.0f, ASPECT);
		mCameraUBO.proj = mpCamera->GetProjMatrix();

		mModelPosition = { 0.0f, 0.0f, 0.0f };
		mModelScale    = 1.0f;
		mModelRotation = { 0.0f, 0.0f, 0.0f };
		mModelMatrix = BuildTransformMatrix(mModelPosition, mModelScale, mModelRotation);

		ShaderCompiler compiler;
		compiler.CompileGLSLShader("basic", mBasicShader);
		/// end  outer resource preparation

		mpContext = Memory::New<VulkanContext>();
		VulkanResourceRegistry::GetInstance()->Initialize(mpContext);

		// create command buffer
		mpCommandBuffers.resize(mpContext->swapchain.imageCount);
		for (auto& pCmdBuf : mpCommandBuffers)
			mpContext->graphicCommandPool->AllocateCommandBuffer(pCmdBuf);

		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,

			-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f
		};

		UInt32 indices[] = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

		CreateGeoBuffers(vertices, indices);
		CreateUBOBuffers();

		mpCameraUBOSetLayout = VulkanDescriptorSetLayout::Builder(mpContext->device)
			.AddBinding(EBufferType::efUniform, 0, 1)
			.Build();
		VulkanDescriptorDataBinder(*mpContext->pDefaultDescriptorPool, *mpCameraUBOSetLayout)
			.BindBuffer(0, VulkanResourceRegistry::GetInstance()->GetBuffer("CameraUniform"))
			.Bind(mpContext->cameraUBOSet);

		// TODO: use shader reflection
		BufferLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat3, "position" },
			{ EShaderDataType::eFloat3, "color" },
		};

		mpPipelineLayout = VulkanPipelineLayout::Builder(mpContext->device)
			.AddDescriptorSetLayout(mpCameraUBOSetLayout)
			.AddPushConstantRange(sizeof(Matrix4f), 0, EShaderStage::efVert)
			.Build();
		mpPipeline = VulkanPipeline::Builder(mpContext->device)
			.SetVertexLayout(vertexBufferLayout)
			.BindLayout(mpPipelineLayout)
			.BindRenderPass(mpContext->renderPass)
			.BindShader(&mBasicShader)
			.Build();

		SG_LOG_INFO("RenderDevice - Vulkan Init");

		BuildRenderGraph();
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpContext->graphicQueue.WaitIdle();

		Memory::Delete(mpPipelineLayout);
		Memory::Delete(mpPipeline);

		Memory::Delete(mpCameraUBOSetLayout);

		VulkanResourceRegistry::GetInstance()->Shutdown();
		Memory::Delete(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Shutdown");

		Memory::Delete(mpCamera);
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		static float totalTime = 0.0f;
		static float speed = 0.005f;
		mModelPosition(0) = 0.5f * Sin(totalTime);
		TranslateToX(mModelMatrix, mModelPosition(0));

		if (mpCamera->IsViewDirty())
		{
			mCameraUBO.view = mpCamera->GetViewMatrix();
			VulkanResourceRegistry::GetInstance()->UpdataBufferData("CameraUniform", &mCameraUBO);
		}

		totalTime += deltaTime * speed;
	}

	void VulkanRenderDevice::OnDraw()
	{
		if (mbWindowMinimal)
			return;

		mpContext->swapchain.AcquireNextImage(mpContext->pPresentCompleteSemaphore, mCurrentFrameInCPU); // check if next image is presented, and get it as the available image
		mpContext->pFences[mCurrentFrameInCPU]->WaitAndReset(); // wait for the render commands running on the new image

		auto& pBuf = mpCommandBuffers[mCurrentFrameInCPU];
		auto* pColorRt = mpContext->colorRts[mCurrentFrameInCPU];

		pBuf.BeginRecord();

		pBuf.SetViewport((float)pColorRt->width, (float)pColorRt->height, 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, (int)pColorRt->width, (int)pColorRt->height });

		ClearValue cv;
		cv.color = { 0.03f, 0.05f, 0.03f, 0.0f };
		cv.depthStencil = { 1.0f, 0 };

		pBuf.BeginRenderPass(mpContext->frameBuffers[mCurrentFrameInCPU], cv);
			pBuf.BindDescriptorSet(mpPipelineLayout, 0, mpContext->cameraUBOSet);
			pBuf.BindPipeline(mpPipeline);

			UInt64 offset[1] = { 0 };
			pBuf.BindVertexBuffer(0, 1, *VulkanResourceRegistry::GetInstance()->GetBuffer("VertexBuffer"), offset);
			pBuf.BindIndexBuffer(*VulkanResourceRegistry::GetInstance()->GetBuffer("IndexBuffer"), 0);

			pBuf.PushConstants(mpPipelineLayout, EShaderStage::efVert, sizeof(Matrix4f), 0, &mModelMatrix);
			pBuf.DrawIndexed(12, 1, 0, 0, 1);
		pBuf.EndRenderPass();
		pBuf.EndRecord();
	
		mpContext->graphicQueue.SubmitCommands(&mpCommandBuffers[mCurrentFrameInCPU], 
			mpContext->pRenderCompleteSemaphore, mpContext->pPresentCompleteSemaphore, mpContext->pFences[mCurrentFrameInCPU]); // submit new render commands to the available image
		mpContext->swapchain.Present(&mpContext->graphicQueue, mCurrentFrameInCPU, mpContext->pRenderCompleteSemaphore); // present the available image

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
			VulkanResourceRegistry::GetInstance()->UpdataBufferData("CameraUniform", &mCameraUBO);
		}
		return true;
	}

	void VulkanRenderDevice::BuildRenderGraph()
	{
		RenderGraphBuilder builder(mRenderGraph);
		//{
		//	auto* pNode = Memory::New<RenderGraphNode>();
		//	pNode->AttachResource("VertexBuffer");
		//	pNode->AttachResource("IndexBuffer");
		//	builder.NewRenderPass(pNode);
		//}
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

		for (auto& pCmdBuf : mpCommandBuffers)
		{
			mpContext->graphicCommandPool->FreeCommandBuffer(pCmdBuf);
			mpContext->graphicCommandPool->AllocateCommandBuffer(pCmdBuf);
		}
	}

	bool VulkanRenderDevice::CreateGeoBuffers(float* vertices, UInt32* indices)
	{
		bool bSuccess = true;

		// vertex buffer
		BufferCreateDesc BufferCI = {};
		BufferCI.name = "VertexBuffer";
		BufferCI.totalSizeInByte = sizeof(float) * 6 * 8;
		BufferCI.type  = EBufferType::efVertex;
		BufferCI.pInitData = vertices;
		bSuccess &= VulkanResourceRegistry::GetInstance()->CreateBuffer(BufferCI, true);

		// index buffer
		BufferCI.name = "IndexBuffer";
		BufferCI.totalSizeInByte = sizeof(UInt32) * 12;
		BufferCI.type = EBufferType::efIndex;
		BufferCI.pInitData = indices;
		bSuccess &= VulkanResourceRegistry::GetInstance()->CreateBuffer(BufferCI, true);

		VulkanResourceRegistry::GetInstance()->FlushBuffers();
		return bSuccess;
	}

	bool VulkanRenderDevice::CreateUBOBuffers()
	{
		BufferCreateDesc BufferCI = {};
		BufferCI.name = "CameraUniform";
		BufferCI.totalSizeInByte = sizeof(UBO);
		BufferCI.type = EBufferType::efUniform;
		return VulkanResourceRegistry::GetInstance()->CreateBuffer(BufferCI);
	}

}