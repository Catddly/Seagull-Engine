#include "StdAfx.h"
#include "Platform/OS.h"

#include "Memory/Memory.h"

#include <windows.h>

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

	static Rect _GetRecommandedWindowRect(Monitor* pMonitor)
	{
		const Int32 width = (Int32)pMonitor->GetMonitorRect().right - (Int32)pMonitor->GetMonitorRect().left;
		const Int32 height = (Int32)pMonitor->GetMonitorRect().bottom - (Int32)pMonitor->GetMonitorRect().top;
		const Int32 shrinkedWidth = width * 4 / 5;
		const Int32 shrinkedHeight = height * 4 / 5;
		Rect rect = {
			(width - shrinkedWidth) / 2,
			(height - shrinkedHeight) / 2,
			width * 4 / 5 + (width - shrinkedWidth) / 2,
			height * 4 / 5 + (height - shrinkedHeight) / 2
		};
		return eastl::move(rect);
	}

	Window::Window(Monitor* const pMonitor, wstring_view name /*= "Mr No Name"*/)
		:mpCurrMonitor(pMonitor), mTitie(name)
	{
		HINSTANCE instance = (HINSTANCE)::GetModuleHandle(NULL);

		HWND hwnd = NULL;
		Rect rect = _GetRecommandedWindowRect(pMonitor);
		RECT r = { rect.left, rect.top, rect.right, rect.bottom };
		DWORD dwExStyle = WS_EX_ACCEPTFILES; // accept drag files.
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		::AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle);

		hwnd = ::CreateWindowEx(dwExStyle, SG_ENGINE_WNAME,
			name.data(), dwStyle, rect.left, rect.top,
			GetRectWidth(rect), GetRectHeight(rect), NULL, NULL,
			instance, 0);

		if (hwnd == NULL) // fail to create window
			SG_ASSERT(false && "Failed to create window!");

		mHandle = hwnd;
		mFullscreenRect = mpCurrMonitor->GetMonitorRect();
		mWindowedRect = rect;

		ShowWindow();
		::UpdateWindow(hwnd);
	}

	Window::~Window()
	{
		::DestroyWindow((HWND)mHandle);
	}

	void Window::SetTitle(const char* title)
	{
		int length = MultiByteToWideChar(CP_ACP, 0, title, -1, NULL, 0);
		WCHAR* buf = reinterpret_cast<WCHAR*>(Memory::Malloc(sizeof(WCHAR) * (length + 1)));
		ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
		MultiByteToWideChar(CP_ACP, 0, title, -1, buf, length);
		mTitie = buf;
		Memory::Free(buf);

		::SetWindowText((HWND)mHandle, mTitie.c_str());
	}

	void Window::AdjustWindow()
	{
		HWND handle = (HWND)mHandle;
		if (mbIsFullScreen)
		{
			// set borderless window
			::SetWindowLong(handle, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN /* no child window draw inside parent*/ |
				WS_CLIPSIBLINGS);

			::SetWindowPos(handle, HWND_NOTOPMOST,
				mFullscreenRect.left, mFullscreenRect.top,
				GetRectWidth(mFullscreenRect), GetRectHeight(mFullscreenRect), SWP_FRAMECHANGED | SWP_NOACTIVATE);
		}
		else
		{
			::SetWindowLong(handle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			::SetWindowLong(handle, GWL_EXSTYLE, WS_EX_ACCEPTFILES);

			::SetWindowPos(handle, HWND_NOTOPMOST,
				mWindowedRect.left,
				mWindowedRect.top,
				GetRectWidth(mWindowedRect),
				GetRectHeight(mWindowedRect),
				SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

			if (mbIsMaximized)
				::ShowWindow(handle, SW_MAXIMIZE);
			else
				::ShowWindow(handle, SW_NORMAL);
		}
	}

	void Window::ShowWindow() const
	{
		::ShowWindow((HWND)mHandle, SW_SHOW);
	}

	void Window::HideWindow() const
	{
		::ShowWindow((HWND)mHandle, SW_HIDE);
	}

	void Window::Resize(const Rect& rect)
	{
		if (mbIsFullScreen)
		{
			if (rect != mFullscreenRect)
			{
				mbIsFullScreen = false;
				mWindowedRect = rect;
				AdjustWindow();
			}
		}
		else
		{
			mWindowedRect = rect;
			AdjustWindow();
		}
	}

	void Window::Resize(UInt32 width, UInt32 height)
	{
		const Int32 w = (Int32)mpCurrMonitor->GetMonitorRect().right;
		const Int32 h = (Int32)mpCurrMonitor->GetMonitorRect().bottom;
		Rect rect = {
			(w - (Int32)width) / 2,
			(h - (Int32)height) / 2,
			(Int32)width + (w - (Int32)width) / 2,
			(Int32)height + (h - (Int32)height) / 2
		};
		Resize(rect);
	}

	void Window::ToggleFullSrceen()
	{
		mbIsFullScreen = !mbIsFullScreen;
		AdjustWindow();
	}

	void Window::Maximized()
	{
		mbIsMaximized = true;
		::ShowWindow((HWND)mHandle, SW_MAXIMIZE);
	}

	void Window::Minimized()
	{
		mbIsMinimized = true;
		::ShowWindow((HWND)mHandle, SW_MAXIMIZE);
	}

	bool Window::IsMinimize() const
	{
		return mbIsMinimized;
	}

	void Window::SetFocus()
	{
		::SetFocus((HWND)mHandle);
		::SetForegroundWindow((HWND)mHandle);
	}

	bool Window::IsFocus()
	{
		HWND handle = ::GetFocus();
		return (handle == mHandle);
	}

	void Window::SetSize(UInt32 width, UInt32 height)
	{
		::SetWindowPos((HWND)mHandle, HWND_TOP, 0, 0,
			(int)width, (int)height, SWP_NOMOVE | SWP_SHOWWINDOW);
	}

	void Window::SetPosition(UInt32 x, UInt32 y)
	{
		::SetWindowPos((HWND)mHandle, HWND_TOP, (int)x, (int)y,
			0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	SG::Vector2i Window::GetMousePosRelative() const
	{
		POINT pos = {};
		::GetCursorPos(&pos);
		::ScreenToClient((HWND)mHandle, &pos);
		Vector2i position = { pos.x, pos.y };
		return eastl::move(position);
	}

	WindowHandle Window::GetNativeHandle()
	{
		return mHandle;
	}

	Rect Window::GetCurrRect()
	{
		if (mbIsFullScreen)
			return mFullscreenRect;
		else
		{
			RECT rect = {};
			::GetWindowRect((HWND)mHandle, &rect);
			mWindowedRect.left   = (Int32)rect.left;
			mWindowedRect.top    = (Int32)rect.top;
			mWindowedRect.right  = (Int32)rect.right;
			mWindowedRect.bottom = (Int32)rect.bottom;
			return mWindowedRect;
		}
	}

	UInt32 Window::GetWidth()
	{
		RECT rect = {};
		::GetClientRect((HWND)mHandle, &rect);
		return rect.right;
	}

	UInt32 Window::GetHeight()
	{
		RECT rect = {};
		::GetClientRect((HWND)mHandle, &rect);
		return rect.bottom;
	}

	float Window::GetAspectRatio()
	{
		return (float)GetWidth() / (float)GetHeight();
	}

}
#endif // SG_PLATFORM_WINDOWS