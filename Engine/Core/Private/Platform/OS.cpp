#include "StdAfx.h"
#include "Platform/OS.h"

#include "System/Logger.h"
#include "System/Input.h"
#include "Profile/Profile.h"

#include "Memory/Memory.h"

namespace SG
{

	DeviceManager OperatingSystem::mDeviceManager;
	WindowManager OperatingSystem::mWindowManager;

	void OperatingSystem::OnInit()
	{
		SG_PROFILE_FUNCTION();

		mDeviceManager.OnInit();
		mWindowManager.OnInit(mDeviceManager.GetPrimaryMonitor());
	}

	void OperatingSystem::OnShutdown()
	{
		SG_PROFILE_FUNCTION();

		mWindowManager.OnShutdown();
		mDeviceManager.OnShutdown();
	}

	SG::Monitor* OperatingSystem::GetMainMonitor()
	{
		return mDeviceManager.GetPrimaryMonitor();
	}

	const UInt32 OperatingSystem::GetAdapterCount()
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

	Window* OperatingSystem::CreateNewWindow()
	{
		SG_PROFILE_FUNCTION();

		return mWindowManager.CreateNewWindow(GetMainMonitor());
	}

	Vector2i OperatingSystem::GetMousePos()
	{
		SG_PROFILE_FUNCTION();

		POINT pos = {};
		::GetCursorPos(&pos);
		Vector2i p = { pos.x, pos.y };
		return eastl::move(p);
	}

	void OperatingSystem::ShowMouseCursor()
	{
		mWindowManager.ShowMouseCursor();
	}

	void OperatingSystem::HideMouseCursor()
	{
		mWindowManager.HideMouseCursor();
	}

	void OperatingSystem::SetMouseCursor(ECursorType type)
	{
		mWindowManager.SetMouseCursor(type);
	}

	bool OperatingSystem::IsMainWindowOutOfScreen()
	{
		SG_PROFILE_FUNCTION();

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