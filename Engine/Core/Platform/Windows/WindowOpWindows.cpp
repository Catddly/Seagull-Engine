#include "StdAfx.h"
#include "WindowOpWindows.h"

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

			SMonitor monitorInfo = {};
			//GetCurrentWindowMonitor(&monitorInfo, pWindow);
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
		return ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	void CWindowsStreamOp::OpenWindow(const WChar* name, SWindow* const pWindow)
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
		wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = L"Seagull Engine";

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

		RECT rect = { 0, 0, 1920, 1080 };
		HWND hwnd = NULL;
		DWORD dwExStyle = WS_EX_ACCEPTFILES; // accept drag files.
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		::AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

		// TODO: replace resolution to recommended resolution.
		hwnd = CreateWindowEx(dwExStyle, L"Seagull Engine",
			name, dwStyle, 0, 0,
			1920, 1080, NULL, NULL,
			instance, 0);

		if (hwnd == NULL) // fail to create window
			SG_ASSERT(false && "Failed to create window!");

		pWindow->handle = hwnd;

		ShowWindow(pWindow);
		::UpdateWindow((HWND)pWindow->handle);
	}

	void CWindowsStreamOp::CloseWindow(SWindow* const pWindow)
	{
		DestroyWindow((HWND)pWindow->handle);
		pWindow->handle = nullptr;
	}

	void CWindowsStreamOp::ShowWindow(SWindow* const pWindow)
	{
		::ShowWindow((HWND)pWindow->handle, SW_SHOW);
	}

	void CWindowsStreamOp::HideWindow(SWindow* const pWindow)
	{
		::ShowWindow((HWND)pWindow->handle, SW_HIDE);
	}

	void CWindowsStreamOp::ResizeWindow(const SRect& rect, SWindow* const pWindow)
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

	void CWindowsStreamOp::ResizeWindow(UInt32 width, UInt32 height, UInt32 center, SWindow* const pWindow)
	{
		SRect rect = { (Int32)center - (Int32)width / 2, (Int32)center - (Int32)height / 2, (Int32)center + (Int32)width / 2, (Int32)center + (Int32)width / 2 };
		ResizeWindow(rect, pWindow);
	}

	void CWindowsStreamOp::ToggleFullSrceen(SWindow* const pWindow)
	{
		pWindow->bIsFullScreen = !pWindow->bIsFullScreen;
		AdjustWindow(pWindow);
	}

	void CWindowsStreamOp::Maximized(SWindow* const pWindow)
	{
		pWindow->bIsMaximized = true;
		::ShowWindow((HWND)pWindow->handle, SW_MAXIMIZE);
	}

	void CWindowsStreamOp::Minimized(SWindow* const pWindow)
	{
		pWindow->bIsMinimized = true;
		::ShowWindow((HWND)pWindow->handle, SW_MINIMIZE);
	}

#endif // SG_PLATFORM_WINDOWS
}