#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "System/FileSystem.h"
#include "System/Logger.h"
#include "Render/SwapChain.h"
#include "Memory/Memory.h"

#include "Math/MathBasic.h"
#include "Math/Plane.h"
#include "Archive/MeshDataArchive.h"
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
// this node draw the debug mesh on the screen.
#include "RendererVulkan/RenderGraphNodes/RGDebugNode.h"
// this node draw the final image and the GUI on the top.
#include "RendererVulkan/RenderGraphNodes/RGFinalOutputNode.h"

#include "RendererVulkan/Renderer/Renderer.h"
#include "RendererVulkan/Renderer/IndirectRenderer.h"
#include "RendererVulkan/Renderer/DebugRenderer.h"

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
		textureCI.mipLevel = texture.mipLevel;

		textureCI.sample = ESampleCount::eSample_1;
		textureCI.usage = EImageUsage::efSample;
		textureCI.type = EImageType::e2D;
		textureCI.format = EImageFormat::eUnorm_R8G8B8A8;
		textureCI.initLayout = EImageLayout::eUndefined;
		textureCI.pInitData = texture.pData;
		textureCI.sizeInByte = texture.sizeInByte;
		VK_RESOURCE()->CreateTexture(textureCI);
		VK_RESOURCE()->FlushTextures();

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

		samplerCI = {};
		samplerCI.name = "texture_1k_mipmap_sampler";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.mipmapMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eRepeat;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = (float)CalcMipmapLevel(1024, 1024);
		samplerCI.enableAnisotropy = true;
		VK_RESOURCE()->CreateSampler(samplerCI);

		CreateVKResourceFromAsset(SSystem()->GetRenderDataBuilder());

		BuildRenderGraph();

		// create all the mesh resource
		IndirectRenderer::OnInit(*mpContext);
		IndirectRenderer::CollectRenderData(SSystem()->GetRenderDataBuilder());

		DebugRenderer::OnInit(*mpContext);

		//mpGUIDriver->PushUserLayer(MakeRef<TestGUILayer>());
		mpDockSpaceGUILayer = MakeRef<DockSpaceLayer>();
		mpGUIDriver->PushUserLayer(mpDockSpaceGUILayer);

		// update one frame here to avoid imgui do not draw the first frame.
		mpGUIDriver->OnUpdate(0.00000001f);

		// wait for all the resource in the transfer queue.
		VK_RESOURCE()->WaitBuffersUpdated();
	}

	void VulkanRenderDevice::OnShutdown()
	{
		SG_PROFILE_FUNCTION();

		mpContext->device.WaitIdle();

		DebugRenderer::OnShutdown();
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

		{
			SG_PROFILE_SCOPE("Listening Events");

			mMessageBusMember.ListenFor("RenderDataRebuild", SG_BIND_MEMBER_FUNC(OnRenderDataRebuild));
			mMessageBusMember.ListenFor<bool>("StatisticsShowDetailChanged", SG_BIND_MEMBER_FUNC(OnShowStatisticsChanged));
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
#if SG_ENABLE_GPU_CULLING
			mpContext->pComputeSyncFences[mCurrentFrame]->WaitAndReset(); // wait for the compute command buffer to finish
#endif
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
#if SG_ENABLE_GPU_CULLING
			VulkanSemaphore* waitSemaphores[2] = { mpContext->pPresentCompleteSemaphore, mpContext->pComputeCompleteSemaphore };
#else
			VulkanSemaphore* waitSemaphores[1] = { mpContext->pPresentCompleteSemaphore };
#endif
#if SG_ENABLE_GPU_CULLING
			EPipelineStage waitStages[2] = { EPipelineStage::efColor_Attachment_Output, EPipelineStage::efCompute_Shader };
#else
			EPipelineStage waitStages[1] = { EPipelineStage::efColor_Attachment_Output };
#endif
#if SG_ENABLE_GPU_CULLING
			mpContext->pGraphicQueue->SubmitCommands<2, 1, 2>(&mpContext->commandBuffers[mCurrentFrame],
				waitStages, signalSemaphores, waitSemaphores,
				mpContext->pFences[mCurrentFrame]);
#else
			mpContext->pGraphicQueue->SubmitCommands<1, 1, 1>(&mpContext->commandBuffers[mCurrentFrame],
				waitStages, signalSemaphores, waitSemaphores,
				mpContext->pFences[mCurrentFrame]);
#endif
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

#if SG_ENABLE_GPU_CULLING
			DrawInfo info;
			info.frameIndex = mCurrentFrame;
			IndirectRenderer::Begin(info);
			IndirectRenderer::WaitForStatisticsCopyed();
			IndirectRenderer::End();
#endif

			// copy statistic data
			auto& statisticData = GetStatisticData();
			statisticData.culledSceneObjects = static_cast<UInt32>(SSystem()->GetMainScene()->GetMeshEntityCount());
#if SG_ENABLE_GPU_CULLING
			auto* pIndirectReadBackBuffer = VK_RESOURCE()->GetBuffer("indirectBuffer_read_back");
			auto* pIndirectCommands = pIndirectReadBackBuffer->MapMemory<DrawIndexedIndirectCommand>();
			for (UInt32 i = 0; i < MeshDataArchive::GetInstance()->GetNumMeshData(); ++i)
				statisticData.culledSceneObjects -= (pIndirectCommands + i)->instanceCount;
			pIndirectReadBackBuffer->UnmapMemory();
#endif

			// copy the query result
			if (!mpContext->pPipelineStatisticsQueryPool->IsSleep())
			{
				auto& [result, bSuccess] = mpContext->pPipelineStatisticsQueryPool->GetQueryResult();
				if (bSuccess)
				memcpy(statisticData.pipelineStatistics.data(), result.data(), sizeof(QueryResult) * result.size());
			}
			if (!mpContext->pTimeStampQueryPool->IsSleep())
			{
				auto& [result, bSuccess] = mpContext->pTimeStampQueryPool->GetQueryResult();
				if (bSuccess)
					statisticData.gpuRenderPassTimes = result;
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
			//.NewRenderPass<RGDebugNode>()
			.NewRenderPass<RGFinalOutputNode>()
			.Build();
	}

	void VulkanRenderDevice::WindowResize()
	{
		SG_PROFILE_FUNCTION();

		mpContext->WindowResize();
		VK_RESOURCE()->WindowResize();
		mpRenderGraph->WindowResize();

		// pop and push the layer to call OnAttach() and OnDetach().
		mpGUIDriver->PopUserLayer(mpDockSpaceGUILayer);
		mpGUIDriver->PushUserLayer(mpDockSpaceGUILayer);
	}

	void VulkanRenderDevice::CreateVKResourceFromAsset(RefPtr<RenderDataBuilder> pRenderDataBuilder)
	{
		SG_PROFILE_FUNCTION();

		bool bHaveTexture = false;

		auto& assets = pRenderDataBuilder->GetCurrentFrameNewAssets();
		for (auto& pWeakRef : assets)
		{
			auto pAsset = pWeakRef.second.lock();
			if (pAsset->GetAssetType() == EAssetType::eTexture)
			{
				TextureAsset* pTextureAsset = static_cast<TextureAsset*>(pAsset.get());
				if (VK_RESOURCE()->HaveTexture(pTextureAsset->GetAssetName().c_str()))
					continue;

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
				else if (pTextureAsset->GetDimention() == 3 || pTextureAsset->GetDimention() == 4)
					texCI.format = EImageFormat::eUnorm_R8G8B8A8;
				texCI.sample = ESampleCount::eSample_1;
				texCI.usage = EImageUsage::efSample;
				texCI.type = EImageType::e2D;
				texCI.pInitData = pTextureAsset->GetRawData();
				texCI.sizeInByte = pTextureAsset->GetByteSize();
				texCI.pUserData = pTextureAsset->GetUserData();
				VK_RESOURCE()->CreateTexture(texCI);

				bHaveTexture |= true;
			}
		}

		if (bHaveTexture)
			VK_RESOURCE()->FlushTextures();

		// release texture raw data
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
	}

	void VulkanRenderDevice::OnRenderDataRebuild()
	{
		auto pRenderDataBuilder = SSystem()->GetRenderDataBuilder();
		CreateVKResourceFromAsset(pRenderDataBuilder);
		IndirectRenderer::CollectRenderData(pRenderDataBuilder);

		VK_RESOURCE()->WaitBuffersUpdated();
	}

	void VulkanRenderDevice::OnShowStatisticsChanged(bool bActive)
	{
		if (bActive)
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

}