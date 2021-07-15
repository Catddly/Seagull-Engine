#pragma once

#include "Common/Render/SwapChain.h"

#include "Common/Platform/IOperatingSystem.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

#define SG_SWAPCHAIN_IMAGE_COUNT 2 // temporary, should be moved to renderer

	struct Renderer;
	class SwapChainVk final : public SwapChain
	{
	public:
		SwapChainVk(EImageFormat format, EPresentMode presentMode, const Resolution& res, Renderer* pRenderer);
		~SwapChainVk();

		//virtual Texture* GetTexture() const override;
	private:
		Renderer*      mpRenderer = nullptr;
		EImageFormat   mFormat = EImageFormat::eNull;
		EPresentMode   mPresentMode = EPresentMode::eImmediate; // can cause tearing
		Resolution     mResolution;

		VkSwapchainKHR  mHandle;
	};


}