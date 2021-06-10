#pragma once

#include "Common/Config.h"
//#include "Common/Base/BasicTypes.h"
#include "../../Common/Base/BasicTypes.h"

#include "Core/Stl/vector.h"
#include "Core/Stl/string.h"

namespace SG
{

	typedef void* WindowHandle;

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
		SRect        windowedRect;    //! Used to save the size of windowed rect, to restore from fullscreen.
		Int32        monitorIndex;    //! Index of the monitor where the current window is located.
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
		SResolution resolution;
		SRect       monitorRect;
		SRect       workRect;
		UInt32      dpiX;
		UInt32      dpiY;
		wstring     name;
	} SMonitor;

	//! Temporary! Need to replace to math system's Vec2.
	typedef struct Vec2
	{
		Float32 x;
		Float32 y;
	} Vec2;

	typedef enum class EOsMessage
	{
		eNull = 0,
		eQuit,
	} EOsMessage;

	/// Operations of window.
	/// To implemented on every platform.
	//! When we open the window, create it.
	SG_COMMON_API void OpenWindow(const WChar* windowName, SWindow* const pWindow);
	//! When we close the window, destroy it.
	SG_COMMON_API void CloseWindow(SWindow* const pWindow);
	//! Collect the information of current window.
	//! @param (pWindow) the window to collect informations. make sure pWindow->handle is not a nullptr.
	//SG_COMMON_API void CollectWindowInfo(SWindow* const pWindow);

	SG_COMMON_API void ShowWindow(SWindow* const pWindow);
	SG_COMMON_API void HideWindow(SWindow* const pWindow);

	SG_COMMON_API void ResizeWindow(const SRect& rect, SWindow* const pWindow);
	SG_COMMON_API void ResizeWindow(UInt32 width, UInt32 height, UInt32 center, SWindow* const pWindow);

	SG_COMMON_API void ToggleFullSrceen(SWindow* const pWindow);
	SG_COMMON_API void Maximized(SWindow* const pWindow);
	SG_COMMON_API void Minimized(SWindow* const pWindow);

	/// Operations of mouse.
	/// To implemented on every platform.
	SG_COMMON_API Vec2 GetMousePosAbsolute();
	SG_COMMON_API Vec2 GetMousePosRelative(SWindow* const pWindow);

	/// Operations of monitor.
	/// To implemented on every platform.
	//! Get the information of the monitors.
	//! @return all the informations of monitors currently active.
	SG_COMMON_API vector<SMonitor> CollectMonitorInfos();
	SG_COMMON_API Vec2 GetDpiScale();

	// TODO: maybe put this on a message bus to transfer all the os messages.
	SG_COMMON_API EOsMessage PeekOSMessage();

	struct IOperatingSystem
	{
		virtual ~IOperatingSystem() = default;

		virtual void OnInit() = 0;
		virtual void OnShutdown() = 0;
	};

}