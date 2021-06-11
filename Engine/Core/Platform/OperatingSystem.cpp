#include "StdAfx.h"
#include "OperatingSystem.h"

#include "Core/Platform/Windows/WindowsDeviceManager.h"
#include "Core/Platform/Windows/WindowsWindowManager.h"

#include "Core/Memory/Memory.h"

namespace SG
{

	void COperatingSystem::OnInit()
	{
#ifdef SG_PLATFORM_WINDOWS
		mDeviceManager = New<CWindowsDeviceManager>();
		mWindowManager = New<CWindowsWindowManager>();
#endif
		mDeviceManager->OnInit();
		mWindowManager->OnInit(mDeviceManager->GetPrimaryMonitor());
		mWindowManager->OpenWindow(L"Test", &mMainWindow);
	}

	void COperatingSystem::OnShutdown()
	{
		mWindowManager->OnShutdown();
		mDeviceManager->OnShutdown();
		Delete(mWindowManager);
		Delete(mDeviceManager);
	}

}