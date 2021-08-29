#pragma once

#include "Defs/Defs.h"
#include "Render/RenderContext.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct Renderer;
	class RendererVk;
	class SG_ALIGN(64) RenderContextVk final : public RenderContext
	{
	public:
		RenderContextVk(Renderer* pRenderer);
		~RenderContextVk();

		virtual Handle GetPhysicalDeviceHandle() const override;
		virtual Handle GetLogicalDeviceHandle() const override;
		virtual Handle GetRenderSurface() const override;
	private:
		//! Fetch the info from the DeviceManager
		//void SelectPhysicalDevice(); // TODO: support multi-GPU
		//void CreateLogicalDevice();
		//void CreateSurface();
		friend class RendererVk;
	private:
		Renderer*        mpRenderer = nullptr;

		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;        //!< corresponding to the adapter in DeviceManager
		VkDevice         mLogicalDevice = VK_NULL_HANDLE;
		VkSurfaceKHR     mPresentSurface = VK_NULL_HANDLE;
	};

}