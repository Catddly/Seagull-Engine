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

	UInt32 COperatingSystem::GetAdapterCount()
	{
		return mDeviceManager.GetAdapterCount();
	}

	Adapter* COperatingSystem::GetPrimaryAdapter()
	{
		return mDeviceManager.GetPrimaryAdapter();
	}

	SG::Window* COperatingSystem::GetMainWindow()
	{
		return mWindowManager.GetMainWindow();
	}

}