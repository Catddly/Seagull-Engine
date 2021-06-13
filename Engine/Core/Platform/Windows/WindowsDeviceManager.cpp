#include "StdAfx.h"
#include "WindowsDeviceManager.h"

#include "Common/Platform/IWindowManager.h"
#include "Common/System/ILog.h"

namespace SG
{

	CWindowsDeviceManager* CWindowsDeviceManager::sInstance = nullptr;

	static BOOL CALLBACK _EnumMonitorCallback(HMONITOR monitor, HDC hdc, LPRECT pRect, LPARAM pUser)
	{
		SMonitor* pMonitor = (SMonitor*)pUser;

		MONITORINFOEXW info = {};
		info.cbSize = sizeof(info);
		::GetMonitorInfoW(monitor, &info);

		pMonitor->monitorRect = { info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right, info.rcMonitor.bottom };
		pMonitor->workRect    = { info.rcWork.left, info.rcWork.top, info.rcWork.right, info.rcWork.bottom };
		return TRUE;
	}

	void CWindowsDeviceManager::OnInit()
	{
		CollectInfos();
	}

	void CWindowsDeviceManager::OnShutdown()
	{
		if (sInstance)
			delete sInstance;
	}

	void CWindowsDeviceManager::CollectInfos()
	{
		// count the monitors
		int monitorCount = 0;
		int adapterCount = 0;
		DISPLAY_DEVICEW adapter = {};
		adapter.cb = sizeof(adapter);
		for (int adapterIndex = 0;; ++adapterIndex)
		{
			if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0)) // if the adapter exists
				break;
			++adapterCount;
			if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
				continue;

			for (int displayIndex = 0;; displayIndex++)
			{
				DISPLAY_DEVICEW display;
				display.cb = sizeof(display);

				if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0)) // if the monitor exists
					break;
				++monitorCount;
			}
		}

		// collect informations for monitor
		mAdapters.resize(adapterCount);
		mMonitors.resize(monitorCount);
		int monitorIndex = 0;
		int adapterIndex = 0;
		if (monitorCount)
		{
			DISPLAY_DEVICEW adapter = {};
			adapter.cb = sizeof(adapter);
			for (int adapterIndex = 0;; ++adapterIndex)
			{
				if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0)) // if the adapter exists
					break;
				SAdapter* pAdapter = &mAdapters[adapterIndex];
				pAdapter->adapterName = adapter.DeviceName;
				pAdapter->name = adapter.DeviceString;
				if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
				{
					pAdapter->bIsActive = false;
					continue;
				}
				pAdapter->bIsActive = true;
				SG_LOG_INFO("Adapter name: %ws", adapter.DeviceString);

				for (int displayIndex = 0;; displayIndex++)
				{
					DISPLAY_DEVICEW display;
					display.cb = sizeof(display);
					if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0)) // if the monitor exists
						break;

					SMonitor* pMonitor = &mMonitors[monitorIndex];
					pMonitor->adapterName = adapter.DeviceName;
					pMonitor->name = display.DeviceString;
					pMonitor->index = monitorIndex;
					SG_LOG_INFO("Monitor name: %ws", pMonitor->name.c_str());
					::EnumDisplayMonitors(NULL, NULL, _EnumMonitorCallback, (LPARAM)(pMonitor));

					// add the current monitor to the adapter
					pAdapter->monitors.emplace_back(pMonitor);

					if ((adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)) // if it is the primary monitor
						pMonitor->bIsPrimary = true;
					else
						pMonitor->bIsPrimary = false;

					++monitorIndex;
				}
			}
		}
		else
		{
			SG_LOG_WARN("No monitor is active or discovered!");
			SG_ASSERT(false);
		}

		// collect all the supported resolutions
		for (auto& e : mMonitors)
		{
			SMonitor* pMonitor = &e;
			DEVMODEW devMode = {};
			devMode.dmSize = sizeof(DEVMODEW);
			devMode.dmFields = DM_PELSHEIGHT | DM_PELSWIDTH;

			EnumDisplaySettingsW(pMonitor->adapterName.c_str(), ENUM_CURRENT_SETTINGS, &devMode);
			pMonitor->defaultResolution.width  = devMode.dmPelsWidth;
			pMonitor->defaultResolution.height = devMode.dmPelsHeight;

			DWORD currentIndex = 0;
			while (EnumDisplaySettingsW(pMonitor->adapterName.c_str(), currentIndex++, &devMode))
			{
				SResolution res = { devMode.dmPelsWidth, devMode.dmPelsHeight };

				bool bIsRepeat = false;
				for (auto& e : pMonitor->resolutions) // get rid of the repeat resolution
				{
					if (e == res)
					{
						bIsRepeat = true;
						break;
					}
				}
				if (bIsRepeat)
					continue;

				SG_LOG_INFO("   resolution: (%d, %d)", res.width, res.height);
				pMonitor->resolutions.emplace_back(res);
			}
		}
	}

	SG::SMonitor* CWindowsDeviceManager::GetCurrWindowMonitor(SWindow* const pWindow)
	{
		HMONITOR currMonitor = {};
		currMonitor = ::MonitorFromWindow((HWND)pWindow->handle, MONITOR_DEFAULTTOPRIMARY);
		if (currMonitor)
		{
			MONITORINFOEXW info;
			info.cbSize = sizeof(info);
			::GetMonitorInfoW(currMonitor, &info);
			for (auto& e : mMonitors)
			{
				if (e.adapterName == info.szDevice)
					return &e;
			}
		}
		return nullptr;
	}

	SG::SMonitor* CWindowsDeviceManager::GetMonitor(UInt32 index)
	{
		for (auto& e : mMonitors)
		{
			if (e.index == index)
				return &e;
		}
		return nullptr;
	}

	SG::SMonitor* CWindowsDeviceManager::GetPrimaryMonitor()
	{
		for (auto& e : mMonitors)
		{
			if (e.bIsPrimary)
				return &e;
		}
		return nullptr;
	}

	SG::CWindowsDeviceManager* CWindowsDeviceManager::GetInstance()
	{
		if (!sInstance) 
			sInstance = new CWindowsDeviceManager;
		return sInstance;
	}

}