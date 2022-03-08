#pragma once

// Seagull Engine embedded the entrypoint inside the engine
// to avoid some user-side problems 
#include "Core/Private/Platform/Main.h"
#include "User/IApp.h"

#include "RendererVulkan/RenderDevice/VulkanRenderDevice.h"

int main(int argv, char** argc)
{
	using namespace SG;
	Main::Initialize();

	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();

	// TODO: replace to runtime binding dll
	SSystem()->RegisterModule<VulkanRenderDevice>();
	Main::AddUserApp(app);
	
	Main::MainLoop();

	SSystem()->UnResgisterModule<VulkanRenderDevice>();
	Main::Shutdown();
}