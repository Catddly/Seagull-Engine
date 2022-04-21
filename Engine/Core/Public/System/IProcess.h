#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"

namespace SG
{
	//! @Interface 
	//! Abstraction of a process
	interface SG_CORE_API IProcess
	{
		virtual ~IProcess() = default;

		virtual void OnInit() = 0;
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnDraw() = 0;
		virtual void OnShutdown() = 0;
	};

}