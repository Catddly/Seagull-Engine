#include "StdAfx.h"
#include "WindowsWindowManager.h"

#include "Common/System/ILog.h"

#include <EASTL/utility.h>

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace SG
{

	CWindowsWindowManager* CWindowsWindowManager::sInstance = nullptr;

	// default window process callback
	static LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

	void CWindowsWindowManager::AdjustWindow(SWindow* const pWindow, SMonitor* const pMonitor)
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

			pWindow->fullscreenRect = pMonitor->monitorRect;
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

	SRect CWindowsWindowManager::GetRecommandedWindowRect(SMonitor* const pMonitor)
	{
		const Int32 width =  (Int32)pMonitor->monitorRect.right;
		const Int32 height = (Int32)pMonitor->monitorRect.bottom;
		const Int32 shrinkedWidth = width * 4 / 5;
		const Int32 shrinkedHeight = height * 4 / 5;
		SRect rect = {
			(width - shrinkedWidth) / 2,
			(height- shrinkedHeight) / 2,
			width * 4 / 5,
			height * 4 / 5
		};
		return eastl::move(rect);
	}

	void CWindowsWindowManager::OnInit(SMonitor* const pMonitor)
	{
		mpCurrMonitor = pMonitor;
		// register a window class
		WNDCLASSEX wc;
		HINSTANCE instance = (HINSTANCE)::GetModuleHandle(NULL);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = _WinProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = instance;
		wc.hIcon = ::LoadIcon(0, IDI_APPLICATION);
		wc.hIconSm = ::LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = ::LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = SG_ENGINE_WNAME;

		if (!::RegisterClassEx(&wc))
		{
			// get the error message, if any.
			DWORD errorMessageID = ::GetLastError();
			if (errorMessageID != ERROR_CLASS_ALREADY_EXISTS)
			{
				LPSTR messageBuffer = NULL;
				Size size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
				string message(messageBuffer, size);
				SG_LOG_ERROR("%s", message.c_str());
				return;
			}
		}
	}

	void CWindowsWindowManager::OnShutdown()
	{
		if (sInstance)
			delete sInstance;
	}

	void CWindowsWindowManager::OpenWindow(const WChar* windowName, SWindow* const pWindow)
	{
		HINSTANCE instance = (HINSTANCE)::GetModuleHandle(NULL);

		HWND hwnd = NULL;
		SRect rect = GetRecommandedWindowRect(mpCurrMonitor);
		RECT r = { rect.left, rect.top, rect.right, rect.bottom };
		DWORD dwExStyle = WS_EX_ACCEPTFILES; // accept drag files.
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		::AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle);

		hwnd = ::CreateWindowEx(dwExStyle, SG_ENGINE_WNAME,
			windowName, dwStyle, rect.left, rect.top,
			rect.right, rect.bottom, NULL, NULL,
			instance, 0);

		if (hwnd == NULL) // fail to create window
			SG_ASSERT(false && "Failed to create window!");

		pWindow->handle = hwnd;
		pWindow->monitorIndex = mpCurrMonitor->index;

		ShowWindow(pWindow);
		::UpdateWindow((HWND)pWindow->handle);
	}

	void CWindowsWindowManager::CloseWindow(SWindow* const pWindow)
	{
		::DestroyWindow((HWND)pWindow->handle);
		pWindow->handle = nullptr;
	}

	void CWindowsWindowManager::ShowWindow(SWindow* const pWindow)
	{
		::ShowWindow((HWND)pWindow->handle, SW_SHOW);
	}

	void CWindowsWindowManager::HideWindow(SWindow* const pWindow)
	{
		::ShowWindow((HWND)pWindow->handle, SW_HIDE);
	}

	void CWindowsWindowManager::ResizeWindow(const SRect& rect, SWindow* const pWindow)
	{
		if (pWindow->bIsFullScreen)
		{
			if (rect != pWindow->fullscreenRect)
			{
				pWindow->bIsFullScreen = false;
				pWindow->windowedRect = rect;
				AdjustWindow(pWindow, mpCurrMonitor);
			}
		}
		else
		{
			pWindow->windowedRect = rect;
			AdjustWindow(pWindow, mpCurrMonitor);
		}
	}

	void CWindowsWindowManager::ResizeWindow(UInt32 width, UInt32 height, UInt32 center, SWindow* const pWindow)
	{
		SRect rect = { (Int32)center - (Int32)width / 2, (Int32)center - (Int32)height / 2, (Int32)center + (Int32)width / 2, (Int32)center + (Int32)width / 2 };
		ResizeWindow(rect, pWindow);
	}

	void CWindowsWindowManager::ToggleFullSrceen(SWindow* const pWindow)
	{
		pWindow->bIsFullScreen = !pWindow->bIsFullScreen;
		AdjustWindow(pWindow, mpCurrMonitor);
	}

	void CWindowsWindowManager::Maximized(SWindow* const pWindow)
	{
		pWindow->bIsMaximized = true;
		::ShowWindow((HWND)pWindow->handle, SW_MAXIMIZE);
	}

	void CWindowsWindowManager::Minimized(SWindow* const pWindow)
	{
		pWindow->bIsMinimized = true;
		::ShowWindow((HWND)pWindow->handle, SW_MINIMIZE);
	}

	Vec2 CWindowsWindowManager::GetMousePosAbsolute() const
	{
		POINT pos = {};
		::GetCursorPos(&pos);
		return { (float)pos.x, (float)pos.y };
	}

	Vec2 CWindowsWindowManager::GetMousePosRelative(SWindow* const pWindow) const
	{
		POINT pos = {};
		::GetCursorPos(&pos);
		ClientToScreen((HWND)pWindow->handle, &pos);
		return { (float)pos.x, (float)pos.y };
	}

	SG::CWindowsWindowManager* CWindowsWindowManager::GetInstance()
	{
		if (!sInstance) 
			sInstance = new CWindowsWindowManager;
		return sInstance;
	}

}