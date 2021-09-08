#pragma once

#include "Defs/Defs.h"
#include "System/IModule.h"

namespace SG
{

#define SG_SWAPCHAIN_IMAGE_COUNT 2 // temporary, should be moved to renderer

	interface IRenderDevice : public IModule
	{
		virtual ~IRenderDevice() = default;
	};

}