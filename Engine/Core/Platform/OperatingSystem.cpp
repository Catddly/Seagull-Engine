#include "StdAfx.h"
#include "OperatingSystem.h"

#include "Core/Platform/Windows/WindowsDeviceManager.h"
#include "Core/Platform/Windows/WindowsWindowManager.h"

#include "Common/System/ILog.h"

namespace SG
{

	void COperatingSystem::OnInit()
	{
#ifdef SG_PLATFORM_WINDOWS
		mDeviceManager = CWindowsDeviceManager::GetInstance();
		mWindowManager = CWindowsWindowManager::GetInstance();
#endif
		mDeviceManager->OnInit();
		mWindowManager->OnInit(mDeviceManager->GetPrimaryMonitor());
		mWindowManager->OpenWindow(SG_ENGINE_WNAME, &mMainWindow);
	}

	void COperatingSystem::OnShutdown()
	{
		mWindowManager->CloseWindow(&mMainWindow);
		mWindowManager->OnShutdown();
		mDeviceManager->OnShutdown();
	}

}