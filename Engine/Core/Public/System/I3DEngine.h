#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"

#include "System/IProcess.h"

namespace SG
{
	//! @Interface
	//! Abstraction of the engine
	interface SG_CORE_API I3DEngine : public IProcess
	{
		virtual ~I3DEngine() = default;
	};

}