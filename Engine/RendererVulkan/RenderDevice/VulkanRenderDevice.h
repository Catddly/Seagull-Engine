#pragma once

#include "RendererVulkan/Config.h"
#include "Render/IRenderDevice.h"

namespace SG
{

	class VulkanInstance;
	class VulkanDevice;
	class VulkanSwapchain;
	class VulkanRenderContext;

	class VulkanRenderDevice : public IRenderDevice
	{
	public:
		VulkanRenderDevice();
		~VulkanRenderDevice();

		SG_RENDERER_VK_API virtual void OnInit();
		SG_RENDERER_VK_API virtual void OnShutdown();

		SG_RENDERER_VK_API virtual void OnUpdate();
		SG_RENDERER_VK_API virtual void OnDraw();

		SG_RENDERER_VK_API virtual const char* GetRegisterName() const { return "RenderDevice"; }

		// TODO: replace it to reflection
		SG_RENDERER_VK_API static const char* GetModuleName() { return "RenderDevice"; }
	protected:
		bool SelectPhysicalDeviceAndCreateDevice();
	private:
		VulkanInstance*      mpInstance = nullptr;
		VulkanDevice*        mpDevice = nullptr;
		VulkanSwapchain*     mpSwapchain = nullptr;
		VulkanRenderContext* mpRenderContext = nullptr;

		//vector<VulkanRenderTarget> mColorRts;
		//VulkanRenderTarget         mDepthRt;

		//ShaderCompiler   mShaderCompiler;

		//VkRenderPass     mDefaultRenderPass = VK_NULL_HANDLE;
		//VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		//VkPipeline       mPipeline = VK_NULL_HANDLE;
		//VkPipelineCache  mPipelineCache = VK_NULL_HANDLE;
	};

}