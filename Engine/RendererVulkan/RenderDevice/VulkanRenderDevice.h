#pragma once

#include "RendererVulkan/Config.h"
#include "Render/IRenderDevice.h"
#include "Render/Shader.h"
#include "Render/GUI/IGUIDriver.h"

#include "Scene/Camera/ICamera.h"
#include "Scene/Scene.h"

#include "System/System.h"

#include "Math/Matrix.h"
#include "stl/vector.h"

namespace SG
{
	class VulkanContext;

	class RenderGraph;

	class VulkanRenderDevice : public IRenderDevice, protected ISystemMessageListener
	{
	public:
		SG_RENDERER_VK_API VulkanRenderDevice();
		SG_RENDERER_VK_API ~VulkanRenderDevice();

		SG_RENDERER_VK_API virtual void OnInit() override;
		SG_RENDERER_VK_API virtual void OnShutdown() override;

		SG_RENDERER_VK_API virtual void OnUpdate(float deltaTime) override;
		SG_RENDERER_VK_API virtual void OnDraw() override;

		SG_RENDERER_VK_API virtual const char* GetRegisterName() const { return "RenderDevice"; }
		// TODO: replace it to reflection
		SG_RENDERER_VK_API static const char* GetModuleName() { return "RenderDevice"; }
	protected:
		virtual bool OnSystemMessage(ESystemMessage msg) override;
		void BuildRenderGraph();
		void WindowResize();

		bool CreateTexture();

		bool MeshToVulkanGeometry();
	private:
		VulkanContext* mpContext = nullptr;

		RenderGraph* mpRenderGraph = nullptr;
		IGUIDriver*  mpGUIDriver = nullptr;

		Scene        mScene;

		UInt32 mCurrentFrameInCPU;
		bool mbBlockEvent = true;
		bool mbWindowMinimal = false;
	};

}