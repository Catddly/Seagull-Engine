#pragma once
#include "../Config.h"

#include "IProcess.h"

namespace SG
{

	struct SG_COMMON_API IApp : public IProcess
	{
		virtual ~IApp() = default;
		//! Initialize before main game loop
		virtual void OnInit() { }
		//! Update per frame
		virtual void OnUpdate() { }
		//! Shutdown the app
		virtual void OnExit() { }
	};

	//! User use this function to return the instance of 
	//! the application to engine
	//! @return user customized app
	SG_COMMON_API IApp* GetAppInstance();
}
