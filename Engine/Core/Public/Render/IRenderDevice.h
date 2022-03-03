#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"

#include "System/Module.h"

namespace SG
{

	interface SG_CORE_API IRenderDevice : public IModule
	{
		IRenderDevice() = default;
		virtual ~IRenderDevice() = default;
	};

}