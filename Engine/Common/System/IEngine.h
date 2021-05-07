#pragma once

#include "Common/System/IProcess.h"

namespace SG
{
	//! @Interface
	//! Abstraction of the engine
	struct IEngine : public IProcess
	{
		//! Temporary test purpose
		virtual void GetMainThreadId() const = 0;
	};

}