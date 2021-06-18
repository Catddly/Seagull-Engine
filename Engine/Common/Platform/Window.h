#pragma once

#include "Common/Base/BasicTypes.h"

#include "Common/Stl/string_view.h"
#include "Common/Math/Vector.h"

namespace SG
{

	typedef void* WindowHandle;

	struct SMonitor;
	struct SRect;
	class Window
	{
	public:
		explicit Window(SMonitor* const pMonitor, wstring_view name = L"Mr No Name");
		~Window();

		void ShowWindow() const;
		void HideWindow() const;

		void ResizeWindow(const SRect& rect);
		void ResizeWindow(UInt32 width, UInt32 height);

		void ToggleFullSrceen();
		void Maximized();
		void Minimized();

		Vector2f GetMousePosRelative() const;
	private:
		void AdjustWindow();
	private:
		WindowHandle mHandle = nullptr;
		SMonitor*    mpCurrMonitor = nullptr;
		bool         bIsFullScreen = false;
		bool         bIsMaximized = false;
		bool         bIsMinimized = false;
		SRect        mFullscreenRect;
		SRect        mWindowedRect;    //! Used to save the size of windowed rect, to restore from full screen.
	};

}