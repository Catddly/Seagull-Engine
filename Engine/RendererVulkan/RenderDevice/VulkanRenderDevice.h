#pragma once

#include "RendererVulkan/Config.h"
#include "Render/IRenderDevice.h"
#include "System/ISystemMessage.h"

#include "RendererVulkan/Shaders/ShaderComiler.h"

#include "stl/vector.h"

namespace SG
{

	class VulkanInstance;
	class VulkanDevice;
	class VulkanSwapchain;
	class VulkanRenderContext;

	interface RenderTarget;
	interface Pipeline;
	interface RenderSemaphore;
	interface RenderFence;
	interface Queue;

	class VulkanRenderDevice : public IRenderDevice, public ISystemMessageListener
	{
	public:
		SG_RENDERER_VK_API VulkanRenderDevice();
		SG_RENDERER_VK_API ~VulkanRenderDevice();

		SG_RENDERER_VK_API virtual void OnInit();
		SG_RENDERER_VK_API virtual void OnShutdown();

		SG_RENDERER_VK_API virtual void OnUpdate();
		SG_RENDERER_VK_API virtual void OnDraw();

		SG_RENDERER_VK_API virtual const char* GetRegisterName() const { return "RenderDevice"; }

		SG_RENDERER_VK_API virtual bool OnSystemMessage(ESystemMessage msg) override;

		// TODO: replace it to reflection
		SG_RENDERER_VK_API static const char* GetModuleName() { return "RenderDevice"; }
	protected:
		bool SelectPhysicalDeviceAndCreateDevice();
		void WindowResize();
		void RecordRenderCommands();
	private:
		bool mbPresentOnce = false;
		bool mbWindowMinimal = false;

		VulkanInstance*      mpInstance = nullptr;
		VulkanDevice*        mpDevice = nullptr;
		VulkanSwapchain*     mpSwapchain = nullptr;
		VulkanRenderContext* mpRenderContext = nullptr;

		vector<RenderTarget*> mpColorRts;
		RenderTarget*         mpDepthRt;

		ShaderCompiler mShaderCompiler;
		Pipeline*      mpPipeline;

		RenderSemaphore*     mpRenderCompleteSemaphore;
		RenderSemaphore*     mpPresentCompleteSemaphore;
		vector<RenderFence*> mpBufferFences;
		Queue*               mpQueue;

		UInt32 mCurrentFrameInCPU;
	};

}