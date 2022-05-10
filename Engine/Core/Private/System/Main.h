#pragma once

#include "Core/Config.h"
#include "User/IApp.h"

namespace SG
{

	//! Abstraction of all the platform main.
	class Main
	{
	public:
		SG_CORE_API static void Initialize();
		SG_CORE_API static void Shutdown();

		SG_CORE_API static void AddUserApp(IApp* pApp);

		SG_CORE_API static void MainLoop();
	};
}