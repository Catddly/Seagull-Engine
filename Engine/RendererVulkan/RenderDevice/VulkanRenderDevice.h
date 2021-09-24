#pragma once

#include "RendererVulkan/Config.h"
#include "Render/IRenderDevice.h"

namespace SG
{

	class VulkanInstance;

	class SG_RENDERER_VK_API VulkanRenderDevice : public IRenderDevice
	{
	public:
		VulkanRenderDevice();
		~VulkanRenderDevice();

		virtual void OnInit();
		virtual void OnShutdown();

		virtual void OnUpdate();
		virtual void OnDraw();

		virtual const char* GetRegisterName() const { return "RenderDevice"; }
	private:
		VulkanInstance* mInstance = nullptr;
	};

}