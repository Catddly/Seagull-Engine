#pragma once

#include "RendererVulkan/Config.h"

#include "System/System.h"
#include "Scene/Camera/ICamera.h"
#include "Event/MessageBus/MessageBus.h"

#include "Render/IRenderDevice.h"
#include "Render/GUI/IGUIDriver.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"
#include "Math/MathBasic.h"

namespace SG
{

	class VulkanContext;
	class RenderGraph;

	class DockSpaceLayer;

	class RenderDataBuilder;

	class VulkanRenderDevice : public IRenderDevice, protected ISystemMessageListener
	{
	public:
		SG_RENDERER_VK_API VulkanRenderDevice();
		SG_RENDERER_VK_API ~VulkanRenderDevice();

		SG_RENDERER_VK_API virtual void OnInit() override;
		SG_RENDERER_VK_API virtual void OnShutdown() override;

		SG_RENDERER_VK_API virtual void OnUpdate(float deltaTime) override;
		SG_RENDERER_VK_API virtual void OnDraw() override;

		SG_RENDERER_VK_API virtual const char* GetRegisterName() const { return "VulkanRenderDevice"; }
	protected:
		virtual bool OnSystemMessage(ESystemMessage msg) override;
		void BuildRenderGraph();
		void WindowResize();

		void CreateVKResourceFromAsset(RefPtr<RenderDataBuilder> pRenderDataBuilder);

		void OnRenderDataRebuild(bool);
		void OnShowStatisticsChanged(bool bActive);
	private:
		VulkanContext* mpContext = nullptr;
		MessageBusMember mMessageBusMember;

		UniquePtr<RenderGraph> mpRenderGraph = nullptr;
		UniquePtr<IGUIDriver>  mpGUIDriver = nullptr;
		UInt32 mCurrentFrame = 0;

		RefPtr<DockSpaceLayer> mpDockSpaceGUILayer = nullptr;

		bool mbBlockEvent = true;
		bool mbWindowMinimal = false;
	};

}