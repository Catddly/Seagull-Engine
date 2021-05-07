#pragma once

//! Seagull Engine embedded the entrypoint inside the engine
//! to avoid some user-side problem

#include "Common/System/ISystem.h"
#include "Common/System/IApp.h"
#include "Engine/Engine/Engine.h"
#include "Log/Log/Log.h"

#include <stdlib.h>

namespace SG
{
	SSystemEnvironment SG_COMMON_API gEnv;
}

int main(int argv, char** argc)
{
	using namespace SG;
	extern IApp* GetAppInstance();
	gEnv.pLog = new CLog;
	gEnv.pEngine = new CEngine;
	IApp* app = SG::GetAppInstance();

	IEngine* engine = gEnv.pEngine;

	engine->OnInit();
	engine->GetMainThreadId();
	app->OnInit();

	char buf[] = "@ILLmew";
	SG_LOG_INFO("Welcome To Seagull Engine! %s", buf);

	engine->OnUpdate();
	app->OnUpdate();

	app->OnExit();
	engine->OnExit();

	system("pause");
}