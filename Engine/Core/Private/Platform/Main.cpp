#include "StdAfx.h"
#include "Core/Private/Platform/Main.h"

#include "System/System.h"

namespace SG
{

	void Main::Initialize()
	{
		SSystem()->Initialize();
		SG_LOG_INFO("Welcome To Seagull Engine!");
	}

	void Main::Shutdown()
	{
		SSystem()->Shutdown();
#if SG_ENABLE_MEMORY_LEAK_DETECTION
		MemoryLeakDetecter::GetInstance()->DumpLeak();
#endif
	}

	void Main::AddUserApp(IApp* pApp)
	{
		SSystem()->AddIProcess(pApp);
	}

	void Main::MainLoop()
	{
		if (SSystem()->SystemMainLoop())
			SG_LOG_INFO("Exit game loop successfully");
		else
			SG_LOG_ERROR("Failed to exit game loop");
	}

}