#include "StdAfx.h"
#include "OperatingSystem.h"

#include "Common/System/ILog.h"

#include "Common/Platform/Window.h"

#include <windows.h>

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

	Vector2i COperatingSystem::GetMousePos() const
	{
		POINT pos = {};
		::GetCursorPos(&pos);
		Vector2i p = { pos.x, pos.y };
		return eastl::move(p);
	}

	bool COperatingSystem::IsMainWindowOutOfScreen() const
	{
		Window* mainWindow = mWindowManager.GetMainWindow();
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