#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "Platform/OS.h"
#include "System/FileSystem.h"
#include "System/Logger.h"
#include "Render/SwapChain.h"
#include "Render/Camera/ICamera.h"
#include "Memory/Memory.h"

#include "Render/Camera/PointOrientedCamera.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"

#include "Archive/ResourceLoader/RenderResourceLoader.h"

// TODO: add graphic api abstraction
#include "RendererVulkan/RenderGraph/RenderGraph.h"
#include "RendererVulkan/RenderGraphNodes/RGDefaultNode.h"
#include "RendererVulkan/RenderGraphNodes/RGEditorGUINode.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "RendererVulkan/GUI/ImGuiDriver.h"

namespace SG
{

	VulkanRenderDevice::VulkanRenderDevice()
		:mCurrentFrameInCPU(0), mbBlockEvent(true)
	{
		SSystem()->RegisterSystemMessageListener(this);
	}

	VulkanRenderDevice::~VulkanRenderDevice()
	{
		SSystem()->RemoveSystemMessageListener(this);
	}

	void VulkanRenderDevice::OnInit()
	{
		/// begin outer resource preparation
		auto* window = OperatingSystem::GetMainWindow();
		const float ASPECT = window->GetAspectRatio();

		mpCamera = Memory::New<PointOrientedCamera>(Vector3f(0.0f, 0.0f, -4.0f));
		mpCamera->SetPerspective(45.0f, ASPECT);
		/// end outer resource preparation

		// use imgui!
		mpGUIDriver = Memory::New<ImGuiDriver>();
		mpGUIDriver->OnInit();

		mpContext = Memory::New<VulkanContext>();
		VK_RESOURCE()->Initialize(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Init");

		//float vertices[] = {
		//	-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		//	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		//	0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		//	-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		//	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f,
		//	0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f,
		//	0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,	2.0f, 0.0f,
		//	-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//};

		//UInt32 indices[] = {
		//	0, 1, 2, 2, 3, 0,
		//	4, 5, 6, 6, 7, 4
		//};

		//CreateGeoBuffers(vertices, indices);
		LoadMeshFromDiskTest();
		//CreateTexture();

		BuildRenderGraph();
		// update one frame here to avoid imgui do not draw the first frame.
		mpGUIDriver->OnUpdate(0.0f);
		mpGUIDriver->OnDraw();
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpContext->graphicQueue.WaitIdle();

		Memory::Delete(mpRenderGraph);
		VK_RESOURCE()->Shutdown();
		Memory::Delete(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Shutdown");

		mpGUIDriver->OnShutdown();
		Memory::Delete(mpGUIDriver);
		Memory::Delete(mpCamera);
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		mpGUIDriver->OnUpdate(deltaTime);
		mpRenderGraph->Update(deltaTime);
	}

	void VulkanRenderDevice::OnDraw()
	{
		if (mbWindowMinimal)
			return;

		mpGUIDriver->OnDraw();

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

	void VulkanRenderDevice::BuildRenderGraph()
	{
		auto* pNode = Memory::New<RGDefaultNode>(*mpContext);
		pNode->BindGeometry("Model");
		pNode->SetCamera(mpCamera);

		mpRenderGraph = RenderGraphBuilder("Default", mpContext)
			.NewRenderPass(pNode)
			.NewRenderPass(Memory::New<RGEditorGUINode>(*mpContext))
			.Build();
	}

	void VulkanRenderDevice::WindowResize()
	{
		mpContext->WindowResize();
		mpRenderGraph->WindowResize();
	}

	bool VulkanRenderDevice::CreateGeoBuffers(float* vertices, UInt32* indices)
	{
		bool bSuccess = true;

		bSuccess &= VK_RESOURCE()->CreateGeometry("square", vertices, 8 * 8, indices, 12);

		return bSuccess;
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
			if (!VK_RESOURCE()->CreateTexture(textureCI, true))
			{
				SG_LOG_ERROR("Failed to create texture!");
				SG_ASSERT(false);
			}
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
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.enableAnisotropy = true;
		VK_RESOURCE()->CreateSampler(samplerCI);

		VK_RESOURCE()->FlushTextures();

		return true;
	}

	bool VulkanRenderDevice::LoadMeshFromDiskTest()
	{
		MeshResourceLoader loader;
		vector<float>  vertices;
		vector<UInt32> indices;
		loader.LoadFromFile("model.obj", vertices, indices);

		return VK_RESOURCE()->CreateGeometry("Model", vertices.data(), static_cast<UInt32>(vertices.size()), 
			indices.data(), static_cast<UInt32>(indices.size()));
	}

}