#pragma once

#include "RendererVulkan/Config.h"
#include "Render/IRenderDevice.h"
#include "Render/Shader.h"

#include "System/Input.h"
#include "System/System.h"
#include "Render/Camera/ICamera.h"

#include "Math/Matrix.h"
#include "stl/vector.h"

namespace SG
{
	class VulkanContext;
	
	class VulkanBuffer;
	class VulkanDescriptorSetLayout;
	class VulkanCommandBuffer;

	struct VulkanPipeline;
	struct VulkanSemaphore;
	struct VulkanFence;
	struct VulkanQueue;
	struct VulkanFrameBuffer;

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
		void WindowResize();
		void RecordRenderCommands();

		bool CreateGeoBuffers(float* vertices, UInt32* indices);
		void DestroyGeoBuffers();

		bool CreateUBOBuffers();
		void DestroyUBOBuffers();
	private:
		bool mbBlockEvent    = true;
		bool mbWindowMinimal = false;
		bool mbUseOrtho = false;

		VulkanContext* mpContext = nullptr;

		vector<VulkanCommandBuffer> mpCommandBuffers;
		VulkanFrameBuffer* mpFrameBuffers = nullptr;

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
		VulkanDescriptorSetLayout* mpCameraUBOSetLayout;
	};

}