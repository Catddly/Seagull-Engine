#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "Stl/string_view.h"
#include "Math/Vector.h"

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

		SG_CORE_API void ShowWindow() const;
		SG_CORE_API void HideWindow() const;

		SG_CORE_API void Resize(const Rect& rect);
		SG_CORE_API void Resize(UInt32 width, UInt32 height);

		SG_CORE_API void ToggleFullSrceen();
		SG_CORE_API void Maximized();
		SG_CORE_API void Minimized();

		SG_CORE_API bool IsMinimize() const;

		SG_CORE_API Rect   GetCurrRect();
		SG_CORE_API UInt32 GetWidth();
		SG_CORE_API UInt32 GetHeight();
		SG_CORE_API Vector2i GetMousePosRelative() const;
		SG_CORE_API WindowHandle GetNativeHandle();
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