#pragma once
#include "../Config.h"

namespace SG
{
	struct IEngine;
	struct ILog;

	//! @Interface 
	//! All the system component is in here
	//! We can dynamically change its implementation of modules
	struct SSystemEnvironment
	{
		IEngine* pEngine = nullptr;
		ILog*    pLog    = nullptr;
	};

}