#pragma once
#include "../Config.h"

namespace SG
{

	struct IEngine;
	struct ILog;

	//! @Interface 
	//! All the system component is in here
	//! We can dynamically change its implementation of modules
	struct ISystemModules
	{
		IEngine* pEngine = nullptr;
		ILog*    pLog = nullptr;
	};

	//! All the system modules are placed here
	//! Before use it you should manually check
	//! if the module is not a nullptr
	SG_COMMON_API ISystemModules gModules;

}