#pragma once

//! Seagull Engine embedded the entrypoint inside the engine
//! to avoid some user-side problems

#include "Common/System/ISystem.h"
#include "Engine/Engine/Engine.h"
#include "Core/Log/Log.h"
#include "Common/User/IApp.h"

#include "Core/Memory/Memory.h"

int main(int argv, char** argc)
{
	using namespace SG;
	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();
	// TODO: replace to seagull's allocator
	gModules.pLog = New<CLog>();
	gModules.p3DEngine = New<CEngine>();
	gModules.pLog->SetFormat("[%y:%o:%d]-[%h:%m:%s]-[%t]");

	gModules.p3DEngine->OnInit();
	app->OnInit();

	char buf[] = "@ILLmew";
	SG_LOG_INFO("Welcome To Seagull Engine! %s", buf);

	gModules.p3DEngine->OnUpdate();
	app->OnUpdate();

	app->OnShutdown();
	gModules.p3DEngine->OnShutdown();

	Delete(gModules.p3DEngine);
	Delete(gModules.pLog);
}