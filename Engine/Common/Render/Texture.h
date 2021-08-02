#pragma once

#include "Common/Render/Renderer.h"
#include "Common/Platform/IOperatingSystem.h"

namespace SG
{

	struct Texture
	{
		virtual ~Texture() = default;

		virtual Resolution GetResolution() const = 0;
		virtual Handle     GetImage()      const = 0;
		virtual Handle     GetImageView()  const = 0;
	};

}