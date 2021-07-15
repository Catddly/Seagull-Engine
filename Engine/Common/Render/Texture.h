#pragma once

#include "Common/Platform/IOperatingSystem.h"

namespace SG
{

	struct Texture
	{
		virtual ~Texture() = default;

		virtual Resolution GetResolution() const = 0;
	};

}