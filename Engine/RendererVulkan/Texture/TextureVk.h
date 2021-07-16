#pragma once

#include "Common/Core/Defs.h"
#include "Common/Render/Texture.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct Renderer;
	class  SwapChainVk;
	class SG_ALIGN(64) TextureVk final : public Texture
	{
	public:
		//! Create an texture by parameters.
		TextureVk(Renderer* pRenderer, const Resolution& res, bool bIsSwapchainImage = false);
		~TextureVk();

		virtual Resolution GetResolution() const override;
	private:
		friend class SwapChainVk;
	private:
		Renderer*  mpRenderer = nullptr;
		Resolution mResolution;
		bool       mbIsSwapchainImage;

		VkImage    mHandle;
	};

}