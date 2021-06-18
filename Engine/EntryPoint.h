#pragma once

//! Seagull Engine embedded the entrypoint inside the engine
//! to avoid some user-side problems

#include "Common/System/ISystem.h"
#include "Common/User/IApp.h"
#include "Common/System/ILog.h"

#include "3DEngine/3DEngine/3DEngine.h"
#include "Core/System/SystemManager.h"

#include "Common/Memory/IMemory.h"

int main(int argv, char** argc)
{
	using namespace SG;
	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();
	ISystemManager* pSystemManager = CSystemManager::GetInstance();
	pSystemManager->InitSystemEnv();
	pSystemManager->InitCoreModules();

	// TOOD: other modules should be loaded as dll,
	// don't use get/set function.
	pSystemManager->AddIProcess(app);
	I3DEngine* p3DEngine = new C3DEngine;
	pSystemManager->SetI3DEngine(p3DEngine);
	p3DEngine->OnInit();

	SG_LOG_IF(ELogLevel::eLog_Level_Info, "Are core modules loaded: ", pSystemManager->ValidateCoreModules());
	SG_LOG_IF(ELogLevel::eLog_Level_Info, "Are all  modules loaded: ", pSystemManager->ValidateAllModules());
	SG_LOG_INFO("Welcome To Seagull Engine!");

	if (pSystemManager->SystemMainLoop())
		SG_LOG_INFO("Exit game loop successfully");
	else
		SG_LOG_ERROR("Failed to exit game loop");

	p3DEngine->OnShutdown();
	delete p3DEngine;
	pSystemManager->Shutdown();
}