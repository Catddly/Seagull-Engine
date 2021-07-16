#pragma once

#include "Common/Render/SwapChain.h"

#include "Common/Platform/IOperatingSystem.h"

#include <vulkan/vulkan_core.h>

#include "Common/Stl/vector.h"

namespace SG
{

	struct Renderer;
	class  TextureVk;
	class SwapChainVk final : public SwapChain
	{
	public:
		SwapChainVk(EImageFormat format, EPresentMode presentMode, const Resolution& res, Renderer* pRenderer);
		~SwapChainVk();

		virtual Texture* GetTexture(UInt32 index) const override;
		virtual Handle   GetNativeHandle() const override;
	private:
		Renderer*      mpRenderer = nullptr;
		EImageFormat   mFormat = EImageFormat::eNull;
		EPresentMode   mPresentMode = EPresentMode::eImmediate; // can cause tearing
		Resolution     mResolution;

		VkSwapchainKHR     mHandle;
		vector<TextureVk*> mImages;
	};


}