#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "System/FileSystem.h"
#include "System/Logger.h"
#include "Render/SwapChain.h"
#include "Memory/Memory.h"

#include "Math/MathBasic.h"
#include "Math/Plane.h"
#include "Scene/Mesh/MeshDataArchive.h"
#include "Profile/Profile.h"
#include "Render/CommonRenderData.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanQueue.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanQueryPool.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"

// TODO: add graphic api abstraction
#include "RendererVulkan/RenderGraph/RenderGraph.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

// this node draw the shadow map and output to the lighting node
#include "RendererVulkan/RenderGraphNodes/RGShadowNode.h"
// this node draw the scene and do lighting calculation
#include "RendererVulkan/RenderGraphNodes/RGDrawSceneNode.h"
// this node draw the scene and do lighting calculation (with IBL-BRDF-PBR)
#include "RendererVulkan/RenderGraphNodes/RGDrawScenePBRNode.h"
// this node draw the final image and the GUI on the top.
#include "RendererVulkan/RenderGraphNodes/RGFinalOutputNode.h"

#include "RendererVulkan/Renderer/Renderer.h"
#include "RendererVulkan/Renderer/IndirectRenderer.h"

#include "RendererVulkan/GUI/ImGuiDriver.h"
#include "RendererVulkan/GUI/TestGUILayer.h"
#include "RendererVulkan/GUI/DockSpaceLayer.h"

namespace SG
{

	VulkanRenderDevice::VulkanRenderDevice()
		:mCurrentFrame(0), mbBlockEvent(true)
	{
		SSystem()->RegisterSystemMessageListener(this);
	}

	VulkanRenderDevice::~VulkanRenderDevice()
	{
		SSystem()->RemoveSystemMessageListener(this);
	}

	void VulkanRenderDevice::OnInit()
	{
		SG_PROFILE_FUNCTION();

		mpGUIDriver = MakeUnique<ImGuiDriver>();
		mpGUIDriver->OnInit();

		mpContext = New(VulkanContext);
		VK_RESOURCE()->Initialize(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Init");

		TextureResourceLoader loader;
		Raw2DTexture texture = {};
		loader.LoadFromFile("logo.png", texture, true);
		TextureCreateDesc textureCI = {};
		textureCI.name = "logo";
		textureCI.width = texture.width;
		textureCI.height = texture.height;
		textureCI.depth = 1;
		textureCI.array = texture.array;
		textureCI.mipLevel = 1;

		textureCI.sample = ESampleCount::eSample_1;
		textureCI.usage = EImageUsage::efSample;
		textureCI.type = EImageType::e2D;
		textureCI.format = EImageFormat::eUnorm_R8G8B8A8;
		textureCI.initLayout = EImageLayout::eUndefined;
		textureCI.pInitData = texture.pData;
		textureCI.sizeInByte = texture.sizeInByte;
		VK_RESOURCE()->CreateTexture(textureCI, true);
		VK_RESOURCE()->FlushTextures();

		CreateVKResourceFromAsset(SSystem()->GetRenderDataBuilder());

		// create all the mesh resource
		IndirectRenderer::OnInit(*mpContext);
		IndirectRenderer::CollectRenderData(SSystem()->GetRenderDataBuilder());

		BuildRenderGraph();

		//mpGUIDriver->PushUserLayer(MakeRef<TestGUILayer>());
		mpDockSpaceGUILayer = MakeRef<DockSpaceLayer>();
		mpGUIDriver->PushUserLayer(mpDockSpaceGUILayer);
		mbCopyStatisticsDetail = mpDockSpaceGUILayer->ShowStatisticsDetail();

		// update one frame here to avoid imgui do not draw the first frame.
		mpGUIDriver->OnUpdate(0.00000001f);
	}

	void VulkanRenderDevice::OnShutdown()
	{
		SG_PROFILE_FUNCTION();

		mpContext->device.WaitIdle();

		IndirectRenderer::OnShutdown();

		mpRenderGraph.reset();
		VK_RESOURCE()->Shutdown();
		Delete(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Shutdown");

		mpGUIDriver->OnShutdown();
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		SG_PROFILE_FUNCTION();

		mpGUIDriver->OnUpdate(deltaTime);
		if (mpDockSpaceGUILayer->ShowStatisticsDetail() != mbCopyStatisticsDetail)
		{
			mbCopyStatisticsDetail = mpDockSpaceGUILayer->ShowStatisticsDetail();
			if (mbCopyStatisticsDetail)
			{
				mpContext->pPipelineStatisticsQueryPool->Wake();
				mpContext->pTimeStampQueryPool->Wake();
			}
			else
			{
				mpContext->pPipelineStatisticsQueryPool->Sleep();
				mpContext->pTimeStampQueryPool->Sleep();
			}
		}

		mpRenderGraph->Update();
		VK_RESOURCE()->OnUpdate();
	}

	void VulkanRenderDevice::OnDraw()
	{
		SG_PROFILE_FUNCTION();

		if (mbWindowMinimal)
			return;

		{
			SG_PROFILE_SCOPE("Acquiring Image");

			mpContext->pSwapchain->AcquireNextImage(mpContext->pPresentCompleteSemaphore, mCurrentFrame); // check if next image is presented, and get it as the available image
		}

		{
			SG_PROFILE_SCOPE("Fence Waiting");

			mpContext->pFences[mCurrentFrame]->WaitAndReset(); // wait for the render commands running on the GPU side to finish
			//mpContext->pComputeSyncFences[mCurrentFrame]->WaitAndReset(); // wait for the compute command buffer to finish
		}

		{
			SG_PROFILE_SCOPE("Render Command Reset");

			mpContext->commandBuffers[mCurrentFrame].Reset();
		}

		mpRenderGraph->Draw(mCurrentFrame);

		{
			SG_PROFILE_SCOPE("Graphic Command Submit");

			// submit graphic commands
			VulkanSemaphore* signalSemaphores[1] = { mpContext->pRenderCompleteSemaphore };
			VulkanSemaphore* waitSemaphores[1] = { mpContext->pPresentCompleteSemaphore };
			//VulkanSemaphore* waitSemaphores[2] = { mpContext->pPresentCompleteSemaphore, mpContext->pComputeCompleteSemaphore };
			EPipelineStage   waitStages[1] = { EPipelineStage::efColor_Attachment_Output };
			//EPipelineStage   waitStages[2] = { EPipelineStage::efColor_Attachment_Output, EPipelineStage::efCompute_Shader };
			mpContext->pGraphicQueue->SubmitCommands<1, 1, 1>(&mpContext->commandBuffers[mCurrentFrame],
				waitStages, signalSemaphores, waitSemaphores,
				mpContext->pFences[mCurrentFrame]);
			// once submit the commands to GPU, pRenderCompleteSemaphore will be locked, and will be unlocked after the GPU finished the commands.
			// we have to wait for the commands had been executed, then we present this image.
			// we use semaphore to have GPU-GPU sync.
		}

		{
			SG_PROFILE_SCOPE("Image Present");

			mpContext->pSwapchain->Present(mpContext->pGraphicQueue, mCurrentFrame, mpContext->pRenderCompleteSemaphore); // present the available image
		}

		{
			SG_PROFILE_SCOPE("Copy Statistics");

			//DrawInfo info;
			//info.frameIndex = mCurrentFrame;
			//IndirectRenderer::Begin(info);
			//IndirectRenderer::WaitForStatisticsCopyed();
			//IndirectRenderer::End();

			//// copy statistic data
			auto& statisticData = GetStatisticData();
			//statisticData.cullSceneObjects = 0;
			//auto* pIndirectReadBackBuffer = VK_RESOURCE()->GetBuffer("indirectBuffer_read_back");
			//auto* pIndirectCommands = pIndirectReadBackBuffer->MapMemory<DrawIndexedIndirectCommand>();
			//for (UInt32 i = 0; i < MeshDataArchive::GetInstance()->GetNumMeshData(); ++i)
			//	statisticData.cullSceneObjects += (pIndirectCommands + i)->instanceCount;
			//pIndirectReadBackBuffer->UnmapMemory();

			// copy the query result
			if (!mpContext->pPipelineStatisticsQueryPool->IsSleep())
			{
				auto& pipelineResult = mpContext->pPipelineStatisticsQueryPool->GetQueryResult();
				memcpy(statisticData.pipelineStatistics.data(), pipelineResult.data(), sizeof(QueryResult) * pipelineResult.size());
			}
			if (!mpContext->pTimeStampQueryPool->IsSleep())
			{
				mpContext->pTimeStampQueryPool->GetQueryResult();
				statisticData.gpuRenderPassTime[0] = mpContext->pTimeStampQueryPool->GetTimeStampDurationMs(0, 1);
				statisticData.gpuRenderPassTime[1] = mpContext->pTimeStampQueryPool->GetTimeStampDurationMs(2, 3);
				statisticData.gpuRenderPassTime[2] = mpContext->pTimeStampQueryPool->GetTimeStampDurationMs(4, 5);
			}
		}

		mbBlockEvent = false;
	}

	bool VulkanRenderDevice::OnSystemMessage(ESystemMessage msg)
	{
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

		mpRenderGraph = RenderGraphBuilder("Default", mpContext)
			.NewRenderPass<RGShadowNode>()
			.NewRenderPass<RGDrawScenePBRNode>()
			.NewRenderPass<RGFinalOutputNode>()
			.Build();
	}

	void VulkanRenderDevice::WindowResize()
	{
		SG_PROFILE_FUNCTION();

		mpContext->WindowResize();
		VK_RESOURCE()->WindowResize();
		mpRenderGraph->WindowResize();

		// pop and push again to attach and detach the layer.
		mpGUIDriver->PopUserLayer(mpDockSpaceGUILayer);
		mpGUIDriver->PushUserLayer(mpDockSpaceGUILayer);
	}

	void VulkanRenderDevice::CreateVKResourceFromAsset(RefPtr<RenderDataBuilder> pRenderDataBuilder)
	{
		SG_PROFILE_FUNCTION();

		bool bHaveTexture = false;

		auto& assets = pRenderDataBuilder->GetAssets();
		for (auto& pWeakRef : assets)
		{
			auto pAsset = pWeakRef.second.lock();
			if (pAsset->GetAssetType() == EAssetType::eTexture)
			{
				TextureAsset* pTextureAsset = static_cast<TextureAsset*>(pAsset.get());
				TextureCreateDesc texCI = {};
				texCI.name = pTextureAsset->GetAssetName().c_str();
				texCI.width = pTextureAsset->GetWidth();
				texCI.height = pTextureAsset->GetHeight();
				texCI.depth = 1;
				texCI.array = pTextureAsset->GetArray();
				texCI.mipLevel = pTextureAsset->GetMipmap();
				if (pTextureAsset->GetDimention() == 1)
					texCI.format = EImageFormat::eUnorm_R8;
				else if (pTextureAsset->GetDimention() == 2)
					texCI.format = EImageFormat::eUnorm_R8G8;
				else if (pTextureAsset->GetDimention() == 3)
					texCI.format = EImageFormat::eUnorm_R8G8B8;
				else if (pTextureAsset->GetDimention() == 4)
					texCI.format = EImageFormat::eUnorm_R8G8B8A8;
				texCI.sample = ESampleCount::eSample_1;
				texCI.usage = EImageUsage::efSample;
				texCI.type = EImageType::e2D;
				texCI.pInitData = pTextureAsset->GetRawData();
				texCI.sizeInByte = pTextureAsset->GetByteSize();
				texCI.pUserData = pTextureAsset->GetUserData();
				VK_RESOURCE()->CreateTexture(texCI, true);

				bHaveTexture |= true;
			}
		}

		if (bHaveTexture)
			VK_RESOURCE()->FlushTextures();

		for (auto& pWeakRef : assets)
		{
			auto pAsset = pWeakRef.second.lock();
			if (pAsset->GetAssetType() == EAssetType::eTexture)
			{
				TextureAsset* pTextureAsset = static_cast<TextureAsset*>(pAsset.get());
				pTextureAsset->FreeMemory();
				SG_ASSERT(!pTextureAsset->IsDiskResourceLoaded());
			}
		}

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "texture_2k_mipmap_sampler";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.mipmapMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eRepeat;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = (float)CalcMipmapLevel(2048, 2048);
		samplerCI.enableAnisotropy = true;
		VK_RESOURCE()->CreateSampler(samplerCI);
	}

}