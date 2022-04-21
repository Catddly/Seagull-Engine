#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "System/FileSystem.h"
#include "System/Logger.h"
#include "Render/SwapChain.h"
#include "Memory/Memory.h"

#include "Math/MathBasic.h"
#include "Math/Plane.h"
#include "Scene/Mesh/MeshDataArchive.h"

#include "RendererVulkan/Backend/VulkanContext.h"
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
		mpGUIDriver = MakeUnique<ImGuiDriver>();
		mpGUIDriver->OnInit();

		mpContext = Memory::New<VulkanContext>();
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
		mpContext->device.WaitIdle();

		IndirectRenderer::OnShutdown();

		mpRenderGraph.reset();
		VK_RESOURCE()->Shutdown();
		Memory::Delete(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Shutdown");

		mpGUIDriver->OnShutdown();
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
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
		if (mbWindowMinimal)
			return;

		mpContext->pSwapchain->AcquireNextImage(mpContext->pPresentCompleteSemaphore, mCurrentFrame); // check if next image is presented, and get it as the available image
		mpContext->pFences[mCurrentFrame]->WaitAndReset(); // wait for the render commands running on the GPU side to finish
		mpContext->pComputeSyncFence->WaitAndReset(); // wait for the compute command buffer to finish

		mpContext->computeCmdBuffer.Reset();
		mpContext->commandBuffers[mCurrentFrame].Reset();

		mpRenderGraph->Draw(mCurrentFrame);

		// submit graphic commands
		mpContext->graphicQueue.SubmitCommands(&mpContext->commandBuffers[mCurrentFrame],
			mpContext->pRenderCompleteSemaphore, mpContext->pPresentCompleteSemaphore, mpContext->pComputeCompleteSemaphore,
			mpContext->pFences[mCurrentFrame]); // submit new render commands to the available image
		// once submit the commands to GPU, pRenderCompleteSemaphore will be locked, and will be unlocked after the GPU finished the commands.
		// we have to wait for the commands had been executed, then we present this image.
		// we use semaphore to have GPU-GPU sync.

		mpContext->pSwapchain->Present(&mpContext->graphicQueue, mCurrentFrame, mpContext->pRenderCompleteSemaphore); // present the available image

		// copy statistic data
		auto& statisticData = GetStatisticData();
		statisticData.cullSceneObjects = 0;
		auto* pIndirectBuffer = VK_RESOURCE()->GetBuffer("indirectBuffer");
		DrawIndexedIndirectCommand* pCommand = pIndirectBuffer->MapMemory<DrawIndexedIndirectCommand>();
		for (UInt32 i = 0; i < MeshDataArchive::GetInstance()->GetNumMeshData(); ++i)
			statisticData.cullSceneObjects += (pCommand + i)->instanceCount;
		pIndirectBuffer->UnmapMemory();
		
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
		mpRenderGraph = RenderGraphBuilder("Default", mpContext)
			.NewRenderPass<RGShadowNode>()
			.NewRenderPass<RGDrawScenePBRNode>()
			.NewRenderPass<RGFinalOutputNode>()
			.Build();
	}

	void VulkanRenderDevice::WindowResize()
	{
		mpContext->WindowResize();
		VK_RESOURCE()->WindowResize();
		mpRenderGraph->WindowResize();
	}

}