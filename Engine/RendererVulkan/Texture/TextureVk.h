#pragma once

#include "Common/Core/Defs.h"
#include "Common/Render/Texture.h"
#include "Common/Render/SwapChain.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct Renderer;
	struct SwapChain;
	class  SwapChainVk;
	class SG_ALIGN(64) TextureVk final : public Texture
	{
	public:
		//! Create an texture by parameters.
		TextureVk(Renderer* pRenderer, const Resolution& res);
		//! Fetch texture from swapchain.
		TextureVk(Renderer* pRenderer, SwapChain* pSwapChain, UInt32 index);
		~TextureVk();

		virtual Resolution GetResolution() const override;
		virtual Handle     GetImage()      const override;
		virtual Handle     GetImageView()  const override;
	private:
		friend class SwapChainVk;
	private:
		Renderer*   mpRenderer = nullptr;
		Resolution  mResolution;
		bool        mbIsSwapchainImage;

		VkImage      mImage;
		VkImageView  mImageView;
		EImageFormat mImageFormat;
	};

}