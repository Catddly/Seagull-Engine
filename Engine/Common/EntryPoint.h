#pragma once

//! Seagull Engine embedded the entrypoint inside the engine
//! to avoid some user-side problem

#include "Common/System/ISystem.h"
#include "Engine/Engine/Engine.h"
#include "Core/Log/Log.h"
#include "Common/User/IApp.h"

#include <stdlib.h>

int main(int argv, char** argc)
{
	using namespace SG;
	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();
	// TODO: replace to seagull's allocator
	gModules.pLog = new CLog;
	gModules.pEngine = new CEngine;
	gModules.pLog->SetFormat("[%y:%o:%d]-[%h:%m:%s]-[%t]");

	gModules.pEngine->OnInit();
	app->OnInit();

	char buf[] = "@ILLmew";
	SG_LOG_INFO("Welcome To Seagull Engine! %s", buf);

	gModules.pEngine->OnUpdate();
	app->OnUpdate();

	app->OnShutdown();
	gModules.pEngine->OnShutdown();

	delete gModules.pEngine;
	delete gModules.pLog;
	system("pause");
}