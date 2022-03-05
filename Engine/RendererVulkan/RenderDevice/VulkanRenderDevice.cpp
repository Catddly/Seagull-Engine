#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "System/FileSystem.h"
#include "System/Logger.h"
#include "Render/SwapChain.h"
#include "Memory/Memory.h"

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
		:mCurrentFrameInCPU(0), mbBlockEvent(true), mScene("default")
	{
		SSystem()->RegisterSystemMessageListener(this);
	}

	VulkanRenderDevice::~VulkanRenderDevice()
	{
		SSystem()->RemoveSystemMessageListener(this);
	}

	void VulkanRenderDevice::OnInit()
	{
		mScene.OnSceneLoad();

		mpGUIDriver = Memory::New<ImGuiDriver>();
		mpGUIDriver->OnInit();

		mpContext = Memory::New<VulkanContext>();
		VK_RESOURCE()->Initialize(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Init");

		MeshToVulkanGeometry();

		BuildRenderGraph();
		// update one frame here to avoid imgui do not draw the first frame.
		mpGUIDriver->OnUpdate(0.0f);
		mpGUIDriver->OnDraw(&mScene);
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mScene.OnSceneUnLoad();
		mpContext->device.WaitIdle();

		Memory::Delete(mpRenderGraph);
		VK_RESOURCE()->Shutdown();
		Memory::Delete(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Shutdown");

		mpGUIDriver->OnShutdown();
		Memory::Delete(mpGUIDriver);
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		//mScene.OnUpdate(deltaTime);
		mpGUIDriver->OnUpdate(deltaTime);
		mpGUIDriver->OnDraw(&mScene);

		mpRenderGraph->Update(deltaTime);
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

	void VulkanRenderDevice::BuildRenderGraph()
	{
		auto* pDefaultNode = Memory::New<RGDefaultNode>(*mpContext);
		pDefaultNode->BindGeometry("model");
		pDefaultNode->SetCamera(mScene.GetMainCamera());
		mScene.TraversePointLight([&](const PointLight& light) 
			{
				pDefaultNode->SetPointLight(&light);
			});

		mpRenderGraph = RenderGraphBuilder("Default", mpContext)
			.NewRenderPass(pDefaultNode)
			.NewRenderPass(Memory::New<RGEditorGUINode>(*mpContext))
			.Build();
	}

	void VulkanRenderDevice::WindowResize()
	{
		mpContext->WindowResize();
		mpRenderGraph->WindowResize();
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

	bool VulkanRenderDevice::MeshToVulkanGeometry()
	{
		bool bAllSuccess = true;
		mScene.TraverseMesh([&](const Mesh& mesh)
			{
				bAllSuccess &= VK_RESOURCE()->CreateGeometry(mesh.GetName(), 
					mesh.GetVertices().data(), static_cast<UInt32>(mesh.GetVertices().size()),
					mesh.GetIndices().data(), static_cast<UInt32>(mesh.GetIndices().size()));
			}
		);
		return bAllSuccess;
	}

}