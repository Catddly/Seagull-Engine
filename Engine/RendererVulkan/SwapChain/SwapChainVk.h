#pragma once

#include "Common/Render/SwapChain.h"

#include "Common/Platform/IOperatingSystem.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	class RendererVk;
	class SwapChainVk final : public SwapChain
	{
	public:
		SwapChainVk(EImageFormat format, EPresentMode presentMode, Resolution res, RendererVk* pRenderer);
		~SwapChainVk();

	private:
		RendererVk*    mpRenderer = nullptr;
		EImageFormat   mFormat = EImageFormat::eNull;
		EPresentMode   mPresentMode = EPresentMode::eImmediate; // can cause tearing
		Resolution     mResolution;
		VkSwapchainKHR mHandle;
	};


}