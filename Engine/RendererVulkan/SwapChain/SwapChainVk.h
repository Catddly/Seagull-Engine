#pragma once

#include "Render/SwapChain.h"

#include "Platform/IOperatingSystem.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"

namespace SG
{

	struct Renderer;
	class  TextureVk;
	class  FrameBufferVk;
	class SwapChainVk final : public Old_SwapChain
	{
	public:
		SwapChainVk(EImageFormat format, EPresentMode presentMode, const Resolution& res, Renderer* pRenderer);
		~SwapChainVk();

		virtual Old_Texture* GetTexture(UInt32 index) const override;
		virtual Handle   GetNativeHandle() const override;

		virtual Resolution GetExtent() const override;

		virtual EImageFormat GetColorFormat() const override;
	private:
		friend class FrameBufferVk;
	private:
		Renderer*      mpRenderer = nullptr;
		EImageFormat   mFormat = EImageFormat::eNull;
		EPresentMode   mPresentMode = EPresentMode::eImmediate; // can cause tearing
		Resolution     mResolution;
		Resolution     mExtent;

		VkSwapchainKHR     mHandle;
		vector<TextureVk*> mImages;
	};


}