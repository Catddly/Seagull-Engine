#include "StdAfx.h"
#include "OperatingSystem.h"

#include "Common/System/ILog.h"

namespace SG
{

	void COperatingSystem::OnInit()
	{
		mDeviceManager.OnInit();
		mWindowManager.OnInit(mDeviceManager.GetPrimaryMonitor());
	}

	void COperatingSystem::OnShutdown()
	{
		mWindowManager.OnShutdown();
		mDeviceManager.OnShutdown();
	}

	SG::Monitor* COperatingSystem::GetMainMonitor()
	{
		return mDeviceManager.GetPrimaryMonitor();
	}

	SG::Window* COperatingSystem::GetMainWindow()
	{
		return mWindowManager.GetMainWindow();
	}

}