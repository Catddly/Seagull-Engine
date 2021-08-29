#pragma once

#include "Render/Renderer.h"

namespace SG
{

	struct RenderContext
	{
		virtual ~RenderContext() = default;

		virtual Handle GetPhysicalDeviceHandle() const = 0;
		virtual Handle GetLogicalDeviceHandle() const = 0;
		virtual Handle GetRenderSurface() const = 0;
	};

}