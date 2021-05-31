#pragma once

//! Seagull Engine embedded the entrypoint inside the engine
//! to avoid some user-side problems

#include "Common/System/ISystem.h"
#include "Common/User/IApp.h"

#include "Core/Log/Log.h"
#include "3DEngine/3DEngine/3DEngine.h"

#include "Core/Memory/Memory.h"

int main(int argv, char** argc)
{
	using namespace SG;
	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();

	gModules.pLog = New<CLog>();
	gModules.p3DEngine = New<C3DEngine>();
	gModules.pLog->SetFormat("[%y:%o:%d]-[%h:%m:%s]");

	gModules.p3DEngine->OnInit();
	app->OnInit();

	gModules.p3DEngine->OnUpdate();
	char buf[] = "@ILLmew";
	SG_LOG_INFO("Welcome To Seagull Engine! %s", buf);
	app->OnUpdate();

	app->OnShutdown();
	gModules.p3DEngine->OnShutdown();
	Delete(gModules.p3DEngine);
	Delete(gModules.pLog);
}