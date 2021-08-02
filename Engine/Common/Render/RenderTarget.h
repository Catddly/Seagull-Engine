#pragma once

#include "Common/Platform/IOperatingSystem.h"

namespace SG
{

	struct RenderTarget
	{
		virtual ~RenderTarget() = default;

		virtual Resolution GetResolution() const = 0;
	};

}