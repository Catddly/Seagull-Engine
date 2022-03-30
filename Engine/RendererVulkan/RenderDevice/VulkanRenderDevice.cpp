#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "System/FileSystem.h"
#include "System/Logger.h"
#include "Render/SwapChain.h"
#include "Memory/Memory.h"

#include "Math/MathBasic.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
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
		mpGUIDriver->PushUserLayer(MakeRef<TestGUILayer>());

		mpContext = Memory::New<VulkanContext>();
		VK_RESOURCE()->Initialize(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Init");

		// create all the mesh resource
		//VK_RESOURCE()->BuildRenderMesh(SSystem()->GetRenderDataBuilder());
		IndirectRenderer::CollectRenderData(SSystem()->GetRenderDataBuilder());

		BuildRenderGraph();
		// update one frame here to avoid imgui do not draw the first frame.
		mpGUIDriver->OnUpdate(0.00000001f);
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpContext->device.WaitIdle();

		mpRenderGraph.reset();
		VK_RESOURCE()->Shutdown();
		Memory::Delete(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Shutdown");

		mpGUIDriver->OnShutdown();
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		VK_RESOURCE()->OnUpdate(SSystem()->GetMainScene());
		mpGUIDriver->OnUpdate(deltaTime);
	}

	void VulkanRenderDevice::OnDraw()
	{
		if (mbWindowMinimal)
			return;

		mpContext->swapchain.AcquireNextImage(mpContext->pPresentCompleteSemaphore, mCurrentFrame); // check if next image is presented, and get it as the available image
		mpContext->pFences[mCurrentFrame]->WaitAndReset(); // wait for the render commands running on the GPU side to finish

		mpContext->commandBuffers[mCurrentFrame].Reset();
		mpRenderGraph->Draw(mCurrentFrame);

		mpContext->graphicQueue.SubmitCommands(&mpContext->commandBuffers[mCurrentFrame],
			mpContext->pRenderCompleteSemaphore, mpContext->pPresentCompleteSemaphore, mpContext->pFences[mCurrentFrame]); // submit new render commands to the available image
		// once submit the commands to GPU, pRenderCompleteSemaphore will be locked, and will be unlocked after the GPU finished the commands.
		// we have to wait for the commands had been executed, then we present this image.
		// we use semaphore to have GPU-GPU sync.
		mpContext->swapchain.Present(&mpContext->graphicQueue, mCurrentFrame, mpContext->pRenderCompleteSemaphore); // present the available image

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
			.NewRenderPass(Memory::New<RGShadowNode>(*mpContext))
			.NewRenderPass(Memory::New<RGDrawScenePBRNode>(*mpContext))
			.NewRenderPass(Memory::New<RGFinalOutputNode>(*mpContext))
			.Build();
	}

	void VulkanRenderDevice::WindowResize()
	{
		mpContext->WindowResize();
		VK_RESOURCE()->WindowResize();
		mpRenderGraph->WindowResize();
	}

}