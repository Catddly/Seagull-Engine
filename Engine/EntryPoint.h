#pragma once

#include "Defs/Defs.h"

// Seagull Engine embedded the entrypoint inside the engine
// to avoid some user-side problems 
#include "Core/Private/System/Main.h"
#include "User/IApp.h"

#include "RendererVulkan/RenderDevice/VulkanRenderDevice.h"

//#ifdef SG_DEBUG
//#	define _CRTDBG_MAP_ALLOC
//#	include <crtdbg.h>
//#endif

int main(int argv, char** argc)
{
	using namespace SG;

	// enable memory leak detection
	// there is some memory leak that i can't find where it is.
//#ifdef SG_DEBUG
//	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
//	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif

	extern IApp* GetAppInstance();
	IApp* app = GetAppInstance();
	Main::AddUserApp(app);

	Main::Initialize();

	// TODO: replace to runtime binding dll
	SSystem()->RegisterModule<VulkanRenderDevice>();
	Main::MainLoop();
	SSystem()->UnResgisterModule<VulkanRenderDevice>();

	Main::Shutdown();

//#ifdef SG_DEBUG
//		_CrtDumpMemoryLeaks();
//#endif
}