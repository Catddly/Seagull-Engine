#pragma once
#include "Common/Config.h"

#include "Common/System/IProcess.h"

namespace SG
{
	//! @Interface
	//! Abstraction of the engine
	struct SG_COMMON_API IEngine : public IProcess
	{
		//! Temporary test purpose
		virtual void GetMainThreadId() const = 0;
	};

}