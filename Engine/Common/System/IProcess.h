#pragma once
#include "../Config.h"

namespace SG
{
	//! @Interface 
	//! Abstraction of a process
	struct SG_COMMON_API IProcess
	{
		virtual ~IProcess() = default;

		virtual void OnInit() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnShutdown() = 0;
	};

}