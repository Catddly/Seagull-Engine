#pragma once

#include "RendererVulkan/Config.h"
#include "Defs/Defs.h"
#include "Render/Renderer.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"

namespace SG
{

#ifdef SG_DEBUG
#	define SG_ENABLE_VK_VALIDATION_LAYER 1
#else
#	define SG_ENABLE_VK_VALIDATION_LAYER 0
#endif

	class QueueVk;
	class SwapChainVk;
	class RenderContextVk;
	class ShaderVk;
	class PipelineVk;
	class FrameBufferVk;
	class RendererVk final : public Renderer
	{
	public:
		SG_RENDERER_VK_API virtual void OnInit() override;
		SG_RENDERER_VK_API virtual void OnShutdown() override;

		//SG_RENDERER_VK_API virtual Handle GetRendererInstance() const override;
		SG_RENDERER_VK_API virtual Queue* GetGraphicQueue() const override;
		SG_RENDERER_VK_API virtual Queue* GetPresentQueue() const override;

		SG_RENDERER_VK_API virtual RenderContext* GetRenderContext() const override;

		SG_RENDERER_VK_API virtual SwapChain* GetSwapChain() const override;

		SG_RENDERER_VK_API virtual const char* GetRegisterName() const override { return "Renderer"; }
	private:
		bool CreateInstance();
		void ValidateExtensions(VkInstanceCreateInfo* info);
		void ValidateLayers(VkInstanceCreateInfo* info);
		void SetupDebugMessager();
		void PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager);
		//! Fetch the info from the DeviceManager
		void SelectPhysicalDevice(); // TODO: support multi-GPU
		void CreateLogicalDevice();
		void CreateSurface();
		void CheckForPresentQueue();
	private:
		VkInstance mInstance = VK_NULL_HANDLE;
		vector<const char*> mValidateExtensions;
		vector<const char*> mValidateLayers;
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		VkDebugUtilsMessengerEXT mDebugMessager = VK_NULL_HANDLE;
#endif
		RenderContextVk* mpRenderContext = nullptr;

		QueueVk* mGraphicQueue = nullptr;
		QueueVk* mPresentQueue = nullptr;
		bool     mbGraphicQueuePresentable = false;

		SwapChainVk* mSwapChain = nullptr;

		// TODO: remove, for temporary test
		ShaderVk*      mBasicShader = nullptr;
		PipelineVk*    mGraphicPipeline = nullptr;
		FrameBufferVk* mFrameBuffer = nullptr;
	};

}