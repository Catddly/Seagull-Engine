#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"

#include "System/IProcess.h"

namespace SG
{
	//! @Interface
	//! Little third-party app for user usage
	interface IApp : public IProcess
	{
		virtual ~IApp() = default;
		//! Initialize before main game loop
		virtual void OnInit() { }
		//! Update per frame
		virtual void OnUpdate() { }
		//! Shutdown the app
		virtual void OnShutdown() { }
	};

	//! User use this function to return the instance of 
	//! the application to engine
	//! @return user customized app
	IApp* GetAppInstance();
}
