#include "StdAfx.h"
#include "Common/Platform/IOSWindow.h"

#include "Common/Core/Defs.h"
#include "Common/System/ILog.h"

#include "Core/Stl/string.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#include <WinUser.h>

namespace SG
{
#ifdef SG_PLATFORM_WINDOWS

	//static DWORD GetDwStyleMask(SWindow* pWindow)
	//{
	//	DWORD style;
	//	if (pWindow->bIsBorderlessWindow)
	//	{
	//		style = WS_POPUP | WS_THICKFRAME;
	//	}
	//	if (pWindow->bIsResizable)
	//	{
	//		style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;
	//	}
	//}

	//static DWORD GetDwExStyleMask(SWindow* pWindow)
	//{

	//}

	/// operations of window
	static void AdjustWindow(SWindow* const pWindow)
	{
		HWND handle = (HWND)pWindow->handle;
		if (pWindow->bIsFullScreen)
		{
			LPRECT currRect = {};
			::GetWindowRect(handle, currRect);
			// save as windowed rect
			pWindow->windowedRect = { currRect->left, currRect->top, currRect->right, currRect->bottom };
			
			// set borderless window
			::SetWindowLong(handle, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN /* no child window draw inside parent*/ |
				WS_CLIPSIBLINGS | WS_THICKFRAME);

			SMonitor monitorInfo;
			GetCurrentWindowMonitor(&monitorInfo, pWindow);
			pWindow->fullscreenRect = monitorInfo.monitorRect;
			::SetWindowPos(handle, HWND_NOTOPMOST,
				pWindow->fullscreenRect.left, pWindow->fullscreenRect.top,
				GetRectWidth(pWindow->fullscreenRect), GetRectHeight(pWindow->fullscreenRect), SWP_FRAMECHANGED | SWP_NOACTIVATE);
		}
		else
		{
			DWORD dwStyle = WS_BORDER | WS_THICKFRAME | WS_VISIBLE;
			DWORD dwExStyle = WS_EX_ACCEPTFILES; // accept drag files.
			::SetWindowLong(handle, GWL_STYLE, dwStyle);
			::SetWindowLong(handle, GWL_EXSTYLE, dwExStyle);

			::SetWindowPos(handle, HWND_NOTOPMOST,
				pWindow->windowedRect.left,
				pWindow->windowedRect.top,
				GetRectWidth(pWindow->windowedRect),
				GetRectHeight(pWindow->windowedRect),
				SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

			if (pWindow->bIsMaximized)
				::ShowWindow(handle, SW_MAXIMIZE);
			else
				::ShowWindow(handle, SW_NORMAL);
		}
	}

	LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return ::DefWindowProc(hwnd, msg, wParam, lParam);;
	}

	void OpenWindow(const WChar* name, SWindow* const pWindow)
	{
		WNDCLASS wc;
		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WinProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = instance;
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = L"MainWnd";

		bool success = RegisterClass(&wc) != 0;

		if (!success)
		{
			// get the error message, if any.
			DWORD errorMessageID = ::GetLastError();

			if (errorMessageID != ERROR_CLASS_ALREADY_EXISTS)
			{
				LPSTR messageBuffer = NULL;
				size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
				string message(messageBuffer, size);
				SG_LOG_ERROR("%s", message.c_str());
				return;
			}
			//else
			//{
			//	gWindowClassInitialized = success;
			//}
		}

		//RECT rect = { 0, 0, 1920, 1080 };
		HWND hwnd = NULL;
		//DWORD dwExStyle = WS_EX_ACCEPTFILES; // accept drag files.
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		//::AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

		// TODO: replace resolution to recommended resolution.
		//hwnd = CreateWindowEx(dwExStyle, L"Seagull Engine",
		//	name, dwStyle, 0, 0,
		//	1920, 1080, NULL, NULL,
		//	instance, 0);
		RECT R = { 0, 0, 1920, 1080 };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);

		hwnd = CreateWindow(L"Seagull Engine",
			name, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
			1920, 1080, NULL, NULL,
			instance, 0);

		if (hwnd == NULL) // fail to create window
			SG_ASSERT(false && "Failed to create window!");

		pWindow->handle = hwnd;
	}

	void CloseWindow(SWindow* const pWindow)
	{
		DestroyWindow((HWND)pWindow->handle);
		pWindow->handle = nullptr;
	}

	void ShowWindow(SWindow* const pWindow)
	{
		::ShowWindow((HWND)pWindow->handle, SW_SHOW);
	}

	void HideWindow(SWindow* const pWindow)
	{
		::ShowWindow((HWND)pWindow->handle, SW_HIDE);
	}

	void ResizeWindow(const SRect& rect, SWindow* const pWindow)
	{
		if (pWindow->bIsFullScreen)
		{
			if (rect != pWindow->fullscreenRect) 
			{
				pWindow->bIsFullScreen = false;
				pWindow->windowedRect = rect;
				AdjustWindow(pWindow);
			}
		}
		else
		{
			pWindow->windowedRect = rect;
			AdjustWindow(pWindow);
		}
	}

	void ResizeWindow(UInt32 width, UInt32 height, UInt32 center, SWindow* const pWindow)
	{
		SRect rect = { center - width / 2, center - height / 2, center + width / 2, center + width / 2 };
		ResizeWindow(rect, pWindow);
	}

	void ToggleFullSrceen(SWindow* const pWindow)
	{
		pWindow->bIsFullScreen = !pWindow->bIsFullScreen;
		AdjustWindow(pWindow);
	}

	void Maximized(SWindow* const pWindow)
	{
		pWindow->bIsMaximized = true;
		::ShowWindow((HWND)pWindow->handle, SW_MAXIMIZE);
	}

	void Minimized(SWindow* const pWindow)
	{
		pWindow->bIsMinimized = true;
		::ShowWindow((HWND)pWindow->handle, SW_MINIMIZE);
	}

	SG::Vec2 GetMousePosAbsolute() 
	{
		POINT pos;
		::GetCursorPos(&pos);
		return { (float)pos.x, (float)pos.y };
	}

	SG::Vec2 GetMousePosRelative(SWindow* const pWindow) 
	{
		POINT pos;
		::GetCursorPos(&pos);
		ClientToScreen((HWND)pWindow->handle, &pos);
		return { (float)pos.x, (float)pos.y };
	}

	/// operations of monitor
	int GetNumMonitors() 
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

	void CollectMonitorInfo(SMonitor* const pMonitor)
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

	Vec2 GetDpiScale()
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

	void GetCurrentWindowMonitor(SMonitor* const pMonitor, SWindow* const pWindow)
	{
		HMONITOR monitor = ::MonitorFromWindow((HWND)pWindow->handle, MONITOR_DEFAULTTOPRIMARY);
		pMonitor->handle = monitor;
		CollectMonitorInfo(pMonitor);
	}


#endif // SG_PLATFORM_WINDOWS
}