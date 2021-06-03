#pragma once

#include "Common/Config.h"

namespace SG
{

	struct I3DEngine;
	struct I2DEngine;
	struct ILog;

	//! @Interface 
	//! All the system component is in here
	//! We can dynamically change its implementation of modules
	struct ISystemModules
	{
		I3DEngine* p3DEngine = nullptr;
		I2DEngine* p2DEngine = nullptr;
		ILog*    pLog = nullptr;
	};

	//! All the system modules are placed here
	//! Before use it you should manually check
	//! if the module is not a nullptr
	SG_COMMON_API ISystemModules gModules;

}