#include "StdAfx.h"
#include "OperatingSystem.h"

#include "Core/Platform/Windows/WindowsDeviceManager.h"

#include "Common/System/ILog.h"

namespace SG
{

	void COperatingSystem::OnInit()
	{
#ifdef SG_PLATFORM_WINDOWS
		mDeviceManager = CWindowsDeviceManager::GetInstance();
#endif
		mDeviceManager->OnInit();
		mWindowManager.OnInit(mDeviceManager->GetPrimaryMonitor());
	}

	void COperatingSystem::OnShutdown()
	{
		mWindowManager.OnShutdown();
		mDeviceManager->OnShutdown();
	}

}