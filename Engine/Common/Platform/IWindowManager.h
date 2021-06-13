#pragma once

#include "../../Common/Base/BasicTypes.h"
#include "IOperatingSystem.h"

namespace SG
{

	typedef void* WindowHandle;

	//! Window information.
	typedef struct SWindow
	{
		WindowHandle handle;
		SRect        fullscreenRect;
		SRect        windowedRect;    //! Used to save the size of windowed rect, to restore from fullscreen.
		Int32        monitorIndex;    //! Index of the monitor where the current window is located.
		bool         bIsFullScreen;
		bool         bIsMaximized;
		bool         bIsMinimized;
	} SWindow;

	struct SMonitor;

	struct IWindowManager
	{
		virtual ~IWindowManager() = default;

		virtual void OnInit(SMonitor* const pMonitor) = 0;
		virtual void OnShutdown() = 0;

		//! When we open the window, create it.
		//! @param (windowName) the name of the new window.
		//! @param (pWindow) necessary info to create the window and return the window instance.
		virtual void OpenWindow(const WChar* windowName, SWindow* const pWindow) = 0;
		//! When we close the window, destroy it.
		virtual void CloseWindow(SWindow* const pWindow) = 0;

		virtual void ShowWindow(SWindow* const pWindow) = 0;
		virtual void HideWindow(SWindow* const pWindow) = 0;

		virtual void ResizeWindow(const SRect& rect, SWindow* const pWindow) = 0;
		virtual void ResizeWindow(UInt32 width, UInt32 height, SWindow* const pWindow) = 0;

		virtual void ToggleFullSrceen(SWindow* const pWindow) = 0;
		virtual void Maximized(SWindow* const pWindow) = 0;
		virtual void Minimized(SWindow* const pWindow) = 0;

		virtual Vec2 GetMousePosAbsolute() const = 0;
		virtual Vec2 GetMousePosRelative(SWindow* const pWindow) const = 0;
	};

}