#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"

#include "System/Module.h"

namespace SG
{

#define SG_SWAPCHAIN_IMAGE_COUNT 3 // temporary, should be moved to renderer

	interface SG_CORE_API IRenderDevice : public IModule
	{
		IRenderDevice() = default;
		virtual ~IRenderDevice() = default;
	};

}