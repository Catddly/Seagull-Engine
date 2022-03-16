#include "StdAfx.h"
#include "Platform/OS.h"

#include "System/Logger.h"

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// DeviceManager
//////////////////////////////////////////////////////////////////////////////////////////////////////

	// used to avoid using friend function in Monitor.
	// TODO: have issues on PC having more than one monitor.
	static Rect tempMonitorRect;
	static Rect tempWorkRect;
	static BOOL _EnumMonitorCallback(HMONITOR monitor, HDC hdc, LPRECT pRect, LPARAM pUser)
	{
		Monitor* pMonitor = (Monitor*)pUser;

		MONITORINFOEXW info = {};
		info.cbSize = sizeof(info);
		::GetMonitorInfoW(monitor, &info);
	
		tempMonitorRect = { info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right, info.rcMonitor.bottom };
		tempWorkRect = { info.rcWork.left, info.rcWork.top, info.rcWork.right, info.rcWork.bottom };
		return TRUE;
	}

	void DeviceManager::OnInit()
	{
		CollectInfos();
	}

	void DeviceManager::OnShutdown()
	{
	}

	void DeviceManager::CollectInfos()
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
		wstring prevAdapterDisplayName = L"";
		mAdapterCount = 0;
		if (monitorCount)
		{
			DISPLAY_DEVICEW adapter = {};
			adapter.cb = sizeof(adapter);
			for (int adapterIndex = 0;; ++adapterIndex)
			{
				if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0)) // if the adapter exists
					break;
				Adapter* pAdapter = &mAdapters[adapterIndex];
				pAdapter->mName = adapter.DeviceName;
				pAdapter->mDisplayName = adapter.DeviceString;
				if (prevAdapterDisplayName != pAdapter->mDisplayName)
				{
					++mAdapterCount;
					prevAdapterDisplayName = pAdapter->mDisplayName;
				}
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

					Monitor* pMonitor = &mMonitors[monitorIndex];
					pMonitor->mAdapterName = adapter.DeviceName;
					pMonitor->mName = display.DeviceString;
					pMonitor->mIndex = monitorIndex;
					SG_LOG_INFO("Monitor name: %ws", pMonitor->mName.c_str());
					::EnumDisplayMonitors(NULL, NULL, _EnumMonitorCallback, (LPARAM)(pMonitor));
					pMonitor->mMonitorRect = tempMonitorRect;
					pMonitor->mWorkRect = tempWorkRect;

					// add the current monitor to the adapter
					pAdapter->mMonitors.emplace_back(pMonitor);

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
			Monitor* pMonitor = &e;
			DEVMODEW devMode = {};
			devMode.dmSize = sizeof(DEVMODEW);
			devMode.dmFields = DM_PELSHEIGHT | DM_PELSWIDTH;

			EnumDisplaySettingsW(pMonitor->mAdapterName.c_str(), ENUM_CURRENT_SETTINGS, &devMode);
			pMonitor->mDefaultResolution.width  = devMode.dmPelsWidth;
			pMonitor->mDefaultResolution.height = devMode.dmPelsHeight;

			DWORD currentIndex = 0;
			while (EnumDisplaySettingsW(pMonitor->mAdapterName.c_str(), currentIndex++, &devMode))
			{
				Resolution res = { devMode.dmPelsWidth, devMode.dmPelsHeight };

				bool bIsRepeat = false;
				for (auto& e : pMonitor->mResolutions) // get rid of the repeat resolution
				{
					if (e == res)
					{
						bIsRepeat = true;
						break;
					}
				}
				if (bIsRepeat)
					continue;

				//SG_LOG_INFO("   resolution: (%d, %d)", res.width, res.height);
				pMonitor->mResolutions.emplace_back(res);
			}
		}
	}

	SG::Monitor* DeviceManager::GetMonitor(UInt32 index) 
	{
		for (auto& e : mMonitors)
		{
			if (e.mIndex == index)
				return &e;
		}
		return nullptr;
	}

	SG::Monitor* DeviceManager::GetPrimaryMonitor()
	{
		for (auto& e : mMonitors)
		{
			if (e.bIsPrimary)
				return &e;
		}
		return nullptr;
	}

	SG::Vector2f DeviceManager::GetDpiScale() const
	{
		HDC hdc = ::GetDC(NULL);
		const float dpiScale = 96.0f; // TODO: maybe this can be set somewhere
		Vector2f dpi;
		if (hdc)
		{
			//dpi.x = (float)::GetDeviceCaps(hdc, LOGPIXELSX) / dpiScale;
			//dpi.y = (float)::GetDeviceCaps(hdc, LOGPIXELSY) / dpiScale;			
			dpi[0] = (float)::GetDeviceCaps(hdc, LOGPIXELSX) / dpiScale;
			dpi[1] = (float)::GetDeviceCaps(hdc, LOGPIXELSY) / dpiScale;
		}
		else
		{
			float systemDpi = ::GetDpiForSystem() / 96.0f;
			dpi[0] = systemDpi;
			dpi[1] = systemDpi;
		}
		::ReleaseDC(NULL, hdc);
		return eastl::move(dpi);
	}

	SG::UInt32 DeviceManager::GetAdapterCount() const
	{
		return mAdapterCount;
	}

	SG::Adapter* DeviceManager::GetPrimaryAdapter()
	{
		for (auto& adapter : mAdapters)
		{
			if (adapter.bIsActive)
				return &adapter;
		}
		return nullptr;
	}

}
#endif // SG_PLATFORM_WINDOWS