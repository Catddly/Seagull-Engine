#include "StdAfx.h"
#include "Platform/OS.h"

#include "System/Logger.h"
#include "System/Input.h"

#include "Memory/Memory.h"

namespace SG
{

	DeviceManager OperatingSystem::mDeviceManager;
	WindowManager OperatingSystem::mWindowManager;

	void OperatingSystem::OnInit()
	{
		mDeviceManager.OnInit();
		mWindowManager.OnInit(mDeviceManager.GetPrimaryMonitor());
	}

	void OperatingSystem::OnShutdown()
	{
		mWindowManager.OnShutdown();
		mDeviceManager.OnShutdown();
	}

	SG::Monitor* OperatingSystem::GetMainMonitor()
	{
		return mDeviceManager.GetPrimaryMonitor();
	}

	UInt32 OperatingSystem::GetAdapterCount()
	{
		return mDeviceManager.GetAdapterCount();
	}

	Adapter* OperatingSystem::GetPrimaryAdapter()
	{
		return mDeviceManager.GetPrimaryAdapter();
	}

	SG::Window* OperatingSystem::GetMainWindow()
	{
		return mWindowManager.GetMainWindow();
	}

	Vector2i OperatingSystem::GetMousePos()
	{
		POINT pos = {};
		::GetCursorPos(&pos);
		Vector2i p = { pos.x, pos.y };
		return eastl::move(p);
	}

	bool OperatingSystem::IsMainWindowOutOfScreen()
	{
		Window*  mainWindow = mWindowManager.GetMainWindow();
		Monitor* mainMonitor = mDeviceManager.GetPrimaryMonitor();
		Vector2i RPos = mainWindow->GetMousePosRelative();
		Vector2i APos = GetMousePos();
		Vector2i rel = RPos - APos;
		Rect windowRect = mainWindow->GetCurrRect();
		Rect monitorRect = mainMonitor->GetMonitorRect();
		if (rel[0] < 0 || rel[1] < 0 || rel[0] + GetRectWidth(windowRect) > GetRectWidth(monitorRect)
			|| rel[1] + GetRectHeight(windowRect) > GetRectHeight(monitorRect))
			return true;
		return false;
	}

}