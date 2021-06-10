#include "StdAfx.h"
#include "OperatingSystem.h"

#include "Core/Platform/DeviceManager.h"

#include "Core/Memory/Memory.h"

namespace SG
{

	void COperatingSystem::OnInit()
	{
		mDeviceManager = New<CDeviceManager>();
		mDeviceManager->OnInit();
		OpenWindow(L"Test", &mMainWindow);
	}

	void COperatingSystem::OnShutdown()
	{
		mDeviceManager->OnShutdown();
		Delete(mDeviceManager);
	}

}