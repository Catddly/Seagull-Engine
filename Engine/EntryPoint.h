#pragma once

// Seagull Engine embedded the entrypoint inside the engine
// to avoid some user-side problems 
#include "Core/Private/Platform/Main.h"
#include "User/IApp.h"

#include "RendererVulkan/RenderDevice/VulkanRenderDevice.h"

int main(int argv, char** argc)
{
	using namespace SG;
	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();
	Main::AddUserApp(app);

	Main::Initialize();
	// TODO: replace to runtime binding dll
	SSystem()->RegisterModule<VulkanRenderDevice>();
	
	Main::MainLoop();

	SSystem()->UnResgisterModule<VulkanRenderDevice>();
	Main::Shutdown();
}