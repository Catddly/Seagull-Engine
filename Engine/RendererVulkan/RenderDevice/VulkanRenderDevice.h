#pragma once

#include "RendererVulkan/Config.h"
#include "Render/IRenderDevice.h"
#include "Render/Shader.h"

#include "System/Input.h"

#include "Render/Camera/ICamera.h"

#include "Math/Matrix.h"

#include "stl/vector.h"

namespace SG
{

	class VulkanInstance;
	class VulkanDevice;
	class VulkanSwapchain;
	class VulkanRenderContext;
	
	struct VulkanRenderTarget;
	struct VulkanPipeline;
	struct VulkanSemaphore;
	struct VulkanFence;
	struct VulkanQueue;
	struct VulkanFrameBuffer;
	struct VulkanBuffer;

	class VulkanRenderDevice : public IRenderDevice, public ISystemMessageListener, public IInputListener
	{
	public:
		SG_RENDERER_VK_API VulkanRenderDevice();
		SG_RENDERER_VK_API ~VulkanRenderDevice();

		SG_RENDERER_VK_API virtual void OnInit() override;
		SG_RENDERER_VK_API virtual void OnShutdown() override;

		SG_RENDERER_VK_API virtual void OnUpdate(float deltaTime) override;
		SG_RENDERER_VK_API virtual void OnDraw() override;

		SG_RENDERER_VK_API virtual const char* GetRegisterName() const { return "RenderDevice"; }

		SG_RENDERER_VK_API virtual bool OnSystemMessage(ESystemMessage msg) override;
		SG_RENDERER_VK_API virtual bool OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState) override;

		// TODO: replace it to reflection
		SG_RENDERER_VK_API static const char* GetModuleName() { return "RenderDevice"; }
	protected:
		bool SelectPhysicalDeviceAndCreateDevice();
		void WindowResize();
		void RecordRenderCommands();

		bool CreateDepthRT();
		void DestroyDepthRT();

		bool CreateBuffers(float* vertices, UInt32* indices);
		void DestroyBuffers();
	private:
		bool mbBlockEvent    = true;
		bool mbWindowMinimal = false;
		bool mbUseOrtho = false;

		VulkanInstance*      mpInstance = nullptr;
		VulkanDevice*        mpDevice = nullptr;
		VulkanSwapchain*     mpSwapchain = nullptr;
		VulkanFrameBuffer*   mpFrameBuffers = nullptr;
		VulkanRenderContext* mpRenderContext = nullptr;

		vector<VulkanRenderTarget*> mpColorRts;
		VulkanRenderTarget*         mpDepthRt;

		VulkanPipeline* mpPipeline;
		Shader          mBasicShader;

		// [GPU 2 GPU Synchronization]
		VulkanSemaphore*     mpRenderCompleteSemaphore;
		VulkanSemaphore*     mpPresentCompleteSemaphore;
		// [CPU 2 GPU Synchronization]
		vector<VulkanFence*> mpBufferFences;

		VulkanQueue*         mpQueue;
		UInt32               mCurrentFrameInCPU;

		VulkanBuffer* mpVertexBuffer;
		VulkanBuffer* mpIndexBuffer;

		// Temporary
		struct SG_ALIGN(64) UBO
		{
			Matrix4f model;
			Matrix4f view;
			Matrix4f proj;
		};

		ICamera* mpCamera;
		UBO      mCameraUBO;
		VulkanBuffer* mpCameraUBOBuffer;
	};

}