#pragma once

// Seagull Engine embedded the entrypoint inside the engine
// to avoid some user-side problems 
#include "System/System.h"
#include "User/IApp.h"

#include "3DEngine/3DEngine/3DEngine.h"
#include "RendererVulkan/RenderDevice/VulkanRenderDevice.h"

#include "Memory/MemoryTracker.h"

int main(int argv, char** argc)
{
	using namespace SG;
	auto* pSystemManager = SSystem();
	pSystemManager->Initialize();

	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();
	//SG_LOG_IF(ELogLevel::efLog_Level_Info, "Are core modules loaded: ", pSystemManager->ValidateCoreModules());

	// TODO: replace to runtime binding dll
	pSystemManager->RegisterModule<VulkanRenderDevice>();
	pSystemManager->AddIProcess(app);

	//SG_LOG_IF(ELogLevel::efLog_Level_Info, "Are all  modules loaded: ", pSystemManager->ValidateAllModules());
	SG_LOG_INFO("Welcome To Seagull Engine!");

	if (pSystemManager->SystemMainLoop())
		SG_LOG_INFO("Exit game loop successfully");
	else
		SG_LOG_ERROR("Failed to exit game loop");

	pSystemManager->Shutdown();
	pSystemManager->UnResgisterModule<VulkanRenderDevice>();
}