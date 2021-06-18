#include "StdAfx.h"
#include "Common/Platform/Window.h"

#include "Common/Platform/IDeviceManager.h"

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

	static SRect _GetRecommandedWindowRect(SMonitor* pMonitor)
	{
		const Int32 width = (Int32)pMonitor->monitorRect.right;
		const Int32 height = (Int32)pMonitor->monitorRect.bottom;
		const Int32 shrinkedWidth = width * 4 / 5;
		const Int32 shrinkedHeight = height * 4 / 5;
		SRect rect = {
			(width - shrinkedWidth) / 2,
			(height - shrinkedHeight) / 2,
			width * 4 / 5 + (width - shrinkedWidth) / 2,
			height * 4 / 5 + (height - shrinkedHeight) / 2
		};
		return eastl::move(rect);
	}

	Window::Window(SMonitor* const pMonitor, wstring_view name /*= "Mr No Name"*/)
		:mpCurrMonitor(pMonitor)
	{
		HINSTANCE instance = (HINSTANCE)::GetModuleHandle(NULL);

		HWND hwnd = NULL;
		SRect rect = _GetRecommandedWindowRect(pMonitor);
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
		mFullscreenRect = mpCurrMonitor->monitorRect;
		mWindowedRect = rect;

		ShowWindow();
		::UpdateWindow(hwnd);
	}

	Window::~Window()
	{

	}

	void Window::AdjustWindow()
	{
		HWND handle = (HWND)mHandle;
		if (bIsFullScreen)
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

			if (bIsMaximized)
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

	void Window::ResizeWindow(const SRect& rect)
	{
		if (bIsFullScreen)
		{
			if (rect != mFullscreenRect)
			{
				bIsFullScreen = false;
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

	void Window::ResizeWindow(UInt32 width, UInt32 height)
	{
		const Int32 w = (Int32)mpCurrMonitor->monitorRect.right;
		const Int32 h = (Int32)mpCurrMonitor->monitorRect.bottom;
		SRect rect = {
			(w - (Int32)width) / 2,
			(h - (Int32)height) / 2,
			(Int32)width + (w - (Int32)width) / 2,
			(Int32)height + (h - (Int32)height) / 2
		};
		ResizeWindow(rect);
	}

	void Window::ToggleFullSrceen()
	{
		bIsFullScreen = !bIsFullScreen;
		AdjustWindow();
	}

	void Window::Maximized()
	{
		bIsMaximized = true;
		::ShowWindow((HWND)mHandle, SW_MAXIMIZE);
	}

	void Window::Minimized()
	{
		bIsMinimized = true;
		::ShowWindow((HWND)mHandle, SW_MAXIMIZE);
	}

	SG::Vector2f Window::GetMousePosRelative() const
	{
		POINT pos = {};
		::GetCursorPos(&pos);
		ClientToScreen((HWND)mHandle, &pos);
		Vector2f position;
		position[0] = (float)pos.x;
		position[1] = (float)pos.y;
		return eastl::move(position);
	}

}
#endif // SG_PLATFORM_WINDOWS