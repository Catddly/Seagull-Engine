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

#include "Archive/ResourceLoader/RenderResourceLoader.h"

// TODO: add graphic api abstraction
#include "RendererVulkan/RenderGraph/RenderGraph.h"
#include "RendererVulkan/RenderGraphNodes/RGShadowNode.h"
#include "RendererVulkan/RenderGraphNodes/RGDefaultNode.h"
#include "RendererVulkan/RenderGraphNodes/RGEditorGUINode.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "RendererVulkan/GUI/ImGuiDriver.h"
#include "RendererVulkan/GUI/TestGUILayer.h"

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
		mpGUIDriver = MakeUnique<ImGuiDriver>();
		mpGUIDriver->OnInit();
		mpGUIDriver->PushUserLayer(MakeRef<TestGUILayer>());

		mpContext = Memory::New<VulkanContext>();
		VK_RESOURCE()->Initialize(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Init");

		// create all the mesh resource
		SSystem()->GetMainScene()->TraverseMesh([](const Mesh& mesh) 
			{
				VK_RESOURCE()->CreateGeometry(mesh.GetName().c_str(),
					mesh.GetVertices().data(), static_cast<UInt32>(mesh.GetVertices().size()),
					mesh.GetIndices().data(), static_cast<UInt32>(mesh.GetIndices().size()));
			});

		BuildRenderGraph();
		// update one frame here to avoid imgui do not draw the first frame.
		mpGUIDriver->OnUpdate(0.0f);
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mpContext->device.WaitIdle();

		mpRenderGraph.reset(nullptr);
		VK_RESOURCE()->Shutdown();
		Memory::Delete(mpContext);
		SG_LOG_INFO("RenderDevice - Vulkan Shutdown");

		mpGUIDriver->OnShutdown();
	}

	void VulkanRenderDevice::OnUpdate(float deltaTime)
	{
		mpGUIDriver->OnUpdate(deltaTime);
		mpRenderGraph->Update();
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
		mpRenderGraph = RenderGraphBuilder("Default", mpContext)
			.NewRenderPass(Memory::New<RGShadowNode>(*mpContext))
			.NewRenderPass(Memory::New<RGDefaultNode>(*mpContext))
			.NewRenderPass(Memory::New<RGEditorGUINode>(*mpContext))
			.Build();
	}

	void VulkanRenderDevice::WindowResize()
	{
		mpContext->WindowResize();
		mpRenderGraph->WindowResize();
	}

}