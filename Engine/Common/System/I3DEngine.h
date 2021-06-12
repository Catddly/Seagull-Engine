#pragma once
#include "Common/Config.h"

#include "Common/System/IProcess.h"

namespace SG
{
	//! @Interface
	//! Abstraction of the engine
	struct SG_COMMON_API I3DEngine : public IProcess
	{
		virtual ~I3DEngine() = default;
	};

}