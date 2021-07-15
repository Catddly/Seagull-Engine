#pragma once

#include "Common/Core/Defs.h"
#include "Common/Render/Texture.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct Renderer;
	class SG_ALIGN(64) TextureVk final : public Texture
	{
	public:
		//! Create an texture by parameters.
		TextureVk(const Resolution& res);
		TextureVk(Renderer* pRenderer, UInt32 index);
		~TextureVk();

		virtual Resolution GetResolution() const override;
	private:
		Resolution mResolution;

		VkImage    mHandle;
	};

}