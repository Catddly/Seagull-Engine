#include "StdAfx.h"
#include "MonitorOpWindows.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace SG
{

	int CWindowsMonitorOp::GetNumMonitors() const
	{
		int numDevices = 0;
		while (true)
		{
			PDISPLAY_DEVICE device = {};
			bool ret = EnumDisplayDevices(NULL, numDevices, device, 0);
			if (ret) // if the device is existed
			{
				++numDevices;
			}
			else
			{
				break;
			}
		}
		return numDevices;
	}

	void CWindowsMonitorOp::CollectMonitorInfo(SMonitor* const pMonitor)
	{
		HMONITOR handle = (HMONITOR)pMonitor->handle;
		MONITORINFOEX info = {};
		info.cbSize = sizeof(MONITORINFOEX);
		::GetMonitorInfo(handle, &info);
		pMonitor->monitorRect = { info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right, info.rcMonitor.bottom };
		pMonitor->workRect = { info.rcWork.left, info.rcWork.top, info.rcWork.right, info.rcWork.bottom };
		pMonitor->resolution = { GetRectWidth(pMonitor->monitorRect), GetRectHeight(pMonitor->monitorRect) };

		Vec2 dpi = GetDpiScale();
		pMonitor->dpiX = (UInt32)dpi.x;
		pMonitor->dpiY = (UInt32)dpi.y;
	}

	Vec2 CWindowsMonitorOp::GetDpiScale() const
	{
		HDC hdc = ::GetDC(NULL);
		const float dpiScale = 96.0f; // TODO: maybe this can be set somewhere
		Vec2 dpi;
		if (hdc)
		{
			dpi.x = static_cast<UINT>(::GetDeviceCaps(hdc, LOGPIXELSX)) / dpiScale;
			dpi.y = static_cast<UINT>(::GetDeviceCaps(hdc, LOGPIXELSY)) / dpiScale;
		}
		else
		{
			float systemDpi = ::GetDpiForSystem() / 96.0f;
			dpi.x = systemDpi;
			dpi.y = systemDpi;
		}
		::ReleaseDC(NULL, hdc);
		return dpi;
	}

	void CWindowsMonitorOp::GetCurrentWindowMonitor(SMonitor* const pMonitor, SWindow* const pWindow)
	{
		HMONITOR monitor = ::MonitorFromWindow((HWND)pWindow->handle, MONITOR_DEFAULTTOPRIMARY);
		pMonitor->handle = monitor;
		CollectMonitorInfo(pMonitor);
	}

}