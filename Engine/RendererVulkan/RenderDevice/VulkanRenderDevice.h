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
	class VulkanPipelineLayout;
	class VulkanPipeline;

	class RenderGraph;

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
		void BuildRenderGraph();
		void WindowResize();

		bool CreateGeoBuffers(float* vertices, UInt32* indices);
		bool CreateUBOBuffers();
		bool CreateTexture();

		bool LoadMeshFromDiskTest();
	private:
		bool mbBlockEvent    = true;
		bool mbWindowMinimal = false;
		bool mbUseOrtho      = false;

		RenderGraph*   mpRenderGraph;

		VulkanContext* mpContext = nullptr;
		ICamera*       mpCamera = nullptr;

		UInt32 mCurrentFrameInCPU;

		VulkanDescriptorSetLayout* mpCameraUBOSetLayout;
		VulkanPipelineLayout* mpPipelineLayout;
		VulkanPipeline*       mpPipeline;
		Shader                mBasicShader;

		// Temporary
		Vector3f mModelPosition;
		float    mModelScale;
		Vector3f mModelRotation;
		Matrix4f mModelMatrix;
		struct SG_ALIGN(64) UBO
		{
			Matrix4f view;
			Matrix4f proj;
		};
		UBO      mCameraUBO;
	};

}