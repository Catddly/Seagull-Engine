#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "Platform/OS.h"
#include "System/FileSystem.h"
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

#include "Archive/ResourceLoader/RenderResourceLoader.h"

// TODO: add graphic api abstraction
#include "RendererVulkan/RenderGraph/RenderGraph.h"
#include "RendererVulkan/RenderGraph/RenderGraphNodes/RGUnlitNode.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

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

		mpCamera = Memory::New<PointOrientedCamera>(Vector3f(0.0f, 0.0f, -4.0f));
		mpCamera->SetPerspective(45.0f, ASPECT);
		mCameraUBO.proj = mpCamera->GetProjMatrix();

		mModelPosition = { 0.0f, 0.0f, 0.0f };
		mModelScale    = 1.0f;
		mModelRotation = { 0.0f, 0.0f, 0.0f };
		mModelMatrix = BuildTransformMatrix(mModelPosition, mModelScale, mModelRotation);

		ShaderCompiler compiler;
		compiler.CompileGLSLShader("basic1", mBasicShader);
		/// end  outer resource preparation

		mpContext = Memory::New<VulkanContext>();
		VK_RESOURCE()->Initialize(mpContext);
		mpRenderGraph = Memory::New<RenderGraph>("Default", mpContext);

		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

			-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f,
			0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f,
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,	2.0f, 0.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		};

		UInt32 indices[] = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

		//CreateGeoBuffers(vertices, indices);
		CreateUBOBuffers();
		CreateTexture();

		LoadMeshFromDiskTest();

		mpCameraUBOSetLayout = VulkanDescriptorSetLayout::Builder(mpContext->device)
			.AddBinding(EDescriptorType::eUniform_Buffer, EShaderStage::efVert, 0, 1)
			//.AddBinding(EDescriptorType::eCombine_Image_Sampler, EShaderStage::efFrag, 1, 1)
			.Build();
		VulkanDescriptorDataBinder(*mpContext->pDefaultDescriptorPool, *mpCameraUBOSetLayout)
			.BindBuffer(0, VK_RESOURCE()->GetBuffer("CameraUniform"))
			//.BindImage(1, VK_RESOURCE()->GetSampler("default"), VK_RESOURCE()->GetTexture("logo"))
			.Bind(mpContext->cameraUBOSet);

		mpPipelineLayout = VulkanPipelineLayout::Builder(mpContext->device)
			.AddDescriptorSetLayout(mpCameraUBOSetLayout)
			.AddPushConstantRange(sizeof(Matrix4f), 0, EShaderStage::efVert)
			.Build();

		SG_LOG_INFO("RenderDevice - Vulkan Init");

		BuildRenderGraph();
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpContext->graphicQueue.WaitIdle();

		Memory::Delete(mpPipelineLayout);
		Memory::Delete(mpCameraUBOSetLayout);

		Memory::Delete(mpRenderGraph);
		VK_RESOURCE()->Shutdown();
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
			VK_RESOURCE()->UpdataBufferData("CameraUniform", &mCameraUBO);
		}

		totalTime += deltaTime * speed;
	}

	void VulkanRenderDevice::OnDraw()
	{
		if (mbWindowMinimal)
			return;

		mpContext->swapchain.AcquireNextImage(mpContext->pPresentCompleteSemaphore, mCurrentFrameInCPU); // check if next image is presented, and get it as the available image
		mpContext->pFences[mCurrentFrameInCPU]->WaitAndReset(); // wait for the render commands running on the new image

		mpRenderGraph->Draw(mCurrentFrameInCPU);

		mpContext->graphicQueue.SubmitCommands(&mpContext->commandBuffers[mCurrentFrameInCPU],
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
			VK_RESOURCE()->UpdataBufferData("CameraUniform", &mCameraUBO);
		}
		return true;
	}

	void VulkanRenderDevice::BuildRenderGraph()
	{
		RenderGraphBuilder builder(*mpRenderGraph);
		{
			auto* pNode = Memory::New<RGUnlitNode>(*mpContext);
			pNode->BindPipeline(mpPipelineLayout, &mBasicShader);
			pNode->BindGeometry("Box");
			pNode->AddDescriptorSet(0, mpContext->cameraUBOSet);
			pNode->AddConstantBuffer(EShaderStage::efVert, sizeof(Matrix4f), &mModelMatrix);

			builder.NewRenderPass(pNode).Complete();
		}
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
		VK_RESOURCE()->UpdataBufferData("CameraUniform", &mCameraUBO);

		mpContext->WindowResize();
		mpRenderGraph->WindowResize();
	}

	bool VulkanRenderDevice::CreateGeoBuffers(float* vertices, UInt32* indices)
	{
		bool bSuccess = true;

		bSuccess &= VK_RESOURCE()->CreateGeometry("square", vertices, 8 * 8, indices, 12);

		VK_RESOURCE()->FlushBuffers();
		return bSuccess;
	}

	bool VulkanRenderDevice::CreateUBOBuffers()
	{
		BufferCreateDesc BufferCI = {};
		BufferCI.name = "CameraUniform";
		BufferCI.totalSizeInByte = sizeof(UBO);
		BufferCI.type = EBufferType::efUniform;
		return VK_RESOURCE()->CreateBuffer(BufferCI);
	}

	bool VulkanRenderDevice::CreateTexture()
	{
		TextureResourceLoader loader;
		Raw2DTexture raw;

		if (loader.LoadFromFile("logo.png", raw))
		{
			TextureCreateDesc textureCI = {};
			textureCI.name = "logo";
			textureCI.width = raw.width;
			textureCI.height = raw.height;
			textureCI.depth = 1;
			textureCI.array = raw.array;
			textureCI.mipLevel = raw.mipLevel;
			textureCI.sizeInByte = raw.width * raw.height * 4;

			textureCI.pInitData = raw.pData;
			textureCI.format = EImageFormat::eUnorm_R8G8B8A8;
			textureCI.sample = ESampleCount::eSample_1;
			textureCI.usage = EImageUsage::efSample;
			textureCI.type = EImageType::e2D;
			VK_RESOURCE()->CreateTexture(textureCI, true);

			VK_RESOURCE()->FlushTextures();
		}
		else
		{
			SG_LOG_WARN("Failed to create the texture named: %s", "logo");
			return false;
		}

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "default";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eRepeat;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 0.0f;
		VK_RESOURCE()->CreateSampler(samplerCI);
		return true;
	}

	bool VulkanRenderDevice::LoadMeshFromDiskTest()
	{
		MeshResourceLoader loader;
		vector<float>  vertices;
		vector<UInt32> indices;
		loader.LoadFromFile("box.obj", vertices, indices);

		return VK_RESOURCE()->CreateGeometry("Box", vertices.data(), static_cast<UInt32>(vertices.size()), 
			indices.data(), static_cast<UInt32>(indices.size()));
	}

}