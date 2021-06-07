#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"
//#include "../../Common/Base/BasicTypes.h"

namespace SG
{

	typedef void* WindowHandle;
	typedef void* MonitorHandle;

	//! Rectangle to indicate an area on screen.
	typedef struct SRect
	{
		Int32 left;
		Int32 top;
		Int32 right;
		Int32 bottom;
		// TODO: support math system.

		bool operator==(const SRect& rhs) const noexcept
		{
			return left == rhs.left && top == rhs.top &&
				right == rhs.right && bottom == rhs.bottom;
		}
		bool operator!=(const SRect& rhs) const noexcept
		{
			return !(*this == rhs);
		}
	} SRect;

	//! Helper function to get the width of the SRect.
	static inline UInt32 GetRectWidth(const SRect& rect)  { return rect.right - rect.left; }
	//! Helper function to get the height of the SRect.
	static inline UInt32 GetRectHeight(const SRect& rect) { return rect.bottom - rect.top; }

	//! Window information.
	typedef struct SWindow
	{
		WindowHandle handle;
		SRect        fullscreenRect;
		SRect        windowedRect;    //! used to save the size of windowed rect, to restore from fullscreen.
		bool         bIsFullScreen;
		bool         bIsResizable;
		bool         bIsMaximized;
		bool         bIsMinimized;
		bool         bIsBorderlessWindow;
	} SWindow;

	//! Resolution in pixel.
	typedef struct SResolution
	{
		UInt32 width;
		UInt32 height;
	} SResolution;

	//! Monitor information.
	typedef struct SMonitor
	{
		MonitorHandle handle;
		SResolution resolution;
		SRect  monitorRect;
		SRect  workRect;
		UInt32 dpiX;
		UInt32 dpiY;
	} SMonitor;

	//! Temporary! Need to replace to math system's Vec2.
	typedef struct Vec2
	{
		Float32 x;
		Float32 y;
	} Vec2;

	// Operations of window.
	// To implemented on every platform.
	//! When we open the window, create it.
	SG_COMMON_API void OpenWindow(const WChar* name, SWindow* const pWindow);
	//! When we close the window, destroy it.
	SG_COMMON_API void CloseWindow(SWindow* const pWindow);

	SG_COMMON_API void ShowWindow(SWindow* const pWindow);
	SG_COMMON_API void HideWindow(SWindow* const pWindow);

	SG_COMMON_API void ResizeWindow(const SRect& rect, SWindow* const pWindow);
	SG_COMMON_API void ResizeWindow(UInt32 width, UInt32 height, UInt32 center, SWindow* const pWindow);

	SG_COMMON_API void ToggleFullSrceen(SWindow* const pWindow);
	SG_COMMON_API void Maximized(SWindow* const pWindow);
	SG_COMMON_API void Minimized(SWindow* const pWindow);

	SG_COMMON_API Vec2 GetMousePosAbsolute();
	SG_COMMON_API Vec2 GetMousePosRelative(SWindow* const pWindow);

	// Operations of monitor.
	// To implemented on every platform.

	//! Get the number of monitors currently connected to the computer.
	SG_COMMON_API int  GetNumMonitors();
	SG_COMMON_API void CollectMonitorInfo(SMonitor* const pMonitor);

	SG_COMMON_API Vec2 GetDpiScale();
	SG_COMMON_API void GetCurrentWindowMonitor(SMonitor* const pMonitor, SWindow* const pWindow);

}