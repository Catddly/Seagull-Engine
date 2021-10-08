#pragma once

#include "Defs/Defs.h"

namespace SG
{

	interface RenderSemaphore
	{
		virtual ~RenderSemaphore() = default;
	};

	interface RenderFence
	{
		virtual ~RenderFence() = default;
	};

}