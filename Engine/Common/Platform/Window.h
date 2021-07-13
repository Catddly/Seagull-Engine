#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

#include "Common/Stl/string_view.h"
#include "Common/Math/Vector.h"

namespace SG
{

	typedef void* WindowHandle;

	class Monitor;
	struct Rect;
	class Window
	{
	public:
		explicit Window(Monitor* const pMonitor, wstring_view name = L"Mr No Name");
		~Window();

		SG_COMMON_API void ShowWindow() const;
		SG_COMMON_API void HideWindow() const;

		SG_COMMON_API void ResizeWindow(const Rect& rect);
		SG_COMMON_API void ResizeWindow(UInt32 width, UInt32 height);

		SG_COMMON_API void ToggleFullSrceen();
		SG_COMMON_API void Maximized();
		SG_COMMON_API void Minimized();

		SG_COMMON_API Rect GetCurrRect() const;

		SG_COMMON_API Vector2f GetMousePosRelative() const;

		SG_COMMON_API WindowHandle GetHandle();
	private:
		void AdjustWindow();
	private:
		WindowHandle mHandle = nullptr;
		Monitor*     mpCurrMonitor = nullptr;
		bool         bIsFullScreen = false;
		bool         bIsMaximized = false;
		bool         bIsMinimized = false;
		Rect         mFullscreenRect;
		Rect         mWindowedRect;    //! Used to save the size of windowed rect, to restore from full screen.
	};

}