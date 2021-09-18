#pragma once

// Seagull Engine embedded the entrypoint inside the engine
// to avoid some user-side problems 
#include "System/System.h"
#include "User/IApp.h"

#include "3DEngine/3DEngine/3DEngine.h"

#include "Memory/IMemory.h"

// TODO: replace to runtime binding dll
#include "RendererVulkan/RenderDevice/RenderDevice.h"

int main(int argv, char** argc)
{
	using namespace SG;
	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();
	auto* pSystemManager = SSystem();
	pSystemManager->OnInit();
	//SG_LOG_IF(ELogLevel::efLog_Level_Info, "Are core modules loaded: ", pSystemManager->ValidateCoreModules());

	// TOOD: other modules should be loaded as dll,
	// don't use get/set function.
	//I3DEngine* p3DEngine = new C3DEngine;
	//pSystemManager->SetI3DEngine(p3DEngine);
	//p3DEngine->OnInit();

	// TODO: replace to runtime binding dll
	pSystemManager->RegisterModule<RenderDeviceVk>();
	pSystemManager->AddIProcess(app);

	//SG_LOG_IF(ELogLevel::efLog_Level_Info, "Are all  modules loaded: ", pSystemManager->ValidateAllModules());
	SG_LOG_INFO("Welcome To Seagull Engine!");

	if (pSystemManager->SystemMainLoop())
		SG_LOG_INFO("Exit game loop successfully");
	else
		SG_LOG_ERROR("Failed to exit game loop");

	pSystemManager->OnShutdown();
}