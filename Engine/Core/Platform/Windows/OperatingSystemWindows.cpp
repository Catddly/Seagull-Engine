#include "StdAfx.h"
#include "Common/Platform/IOperatingSystem.h"

#include "Common/Core/Defs.h"
#include "Common/System/ILog.h"

#include "Core/Stl/string.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace SG
{
#ifdef SG_PLATFORM_WINDOWS
	///////////////////////////////////////////////////////////////////////////
	///  global functions of operating system
	///////////////////////////////////////////////////////////////////////////

	Vec2 GetCurrDpiScale()
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

	/// mouse functions
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

	/// window functions
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

			//SMonitor monitorInfo = {};
			//GetWindowMonitor(&monitorInfo, pWindow); // TODO: encapsulate monitor information collecting.
			pWindow->fullscreenRect = { 500, 500, 2560, 1440 };
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
		switch (msg)
		{
		case WM_DESTROY:
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			break;
		}
		}
		return ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	void OpenWindow(const WChar* name, SWindow* const pWindow)
	{
		WNDCLASSEX wc;
		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WinProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = instance;
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = SG_ENGINE_WNAME;

		if (!RegisterClassEx(&wc))
		{
			// get the error message, if any.
			DWORD errorMessageID = ::GetLastError();
			if (errorMessageID != ERROR_CLASS_ALREADY_EXISTS)
			{
				LPSTR messageBuffer = NULL;
				Size size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
				string message(messageBuffer, size);
				SG_LOG_ERROR("%s", message.c_str());
				return;
			}
		}

		RECT rect = { (2560 - 1920) / 2, (1440 - 1080) / 2, 1920, 1080 };
		HWND hwnd = NULL;
		DWORD dwExStyle = WS_EX_ACCEPTFILES; // accept drag files.
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		::AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

		// TODO: replace resolution to recommended resolution.
		hwnd = CreateWindowEx(dwExStyle, SG_ENGINE_WNAME,
			name, dwStyle, (2560 - 1920) / 2, (1440 - 1080) / 2,
			1920, 1080, NULL, NULL,
			instance, 0);

		if (hwnd == NULL) // fail to create window
			SG_ASSERT(false && "Failed to create window!");

		pWindow->handle = hwnd;

		ShowWindow(pWindow);
		::UpdateWindow((HWND)pWindow->handle);
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
		SRect rect = { (Int32)center - (Int32)width / 2, (Int32)center - (Int32)height / 2, (Int32)center + (Int32)width / 2, (Int32)center + (Int32)width / 2 };
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

	EOsMessage PeekOSMessage()
	{
		MSG msg = {};
		EOsMessage osMeg = EOsMessage::eNull;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);

			if (WM_CLOSE == msg.message || WM_QUIT == msg.message)
				osMeg = EOsMessage::eQuit;
		}
		return osMeg;
	}

	///////////////////////////////////////////////////////////////////////////
	///  global functions of operating system
	///////////////////////////////////////////////////////////////////////////
#endif
}