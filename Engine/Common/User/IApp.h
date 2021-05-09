#pragma once
#include "../Config.h"

#include "Common/System/IProcess.h"

namespace SG
{
	//! @Interface
	//! Little third-party app for user usage
	struct IApp : public IProcess
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
