#pragma once

#include "Common/Platform/IOperatingSystem.h"

namespace SG
{

	struct CWindowsStreamOp : public IWindowOp
	{
		//! When we open the window, create it.
		virtual void OpenWindow(const WChar* windowName, SWindow* const pWindow) override;
		//! When we close the window, destroy it.
		virtual void CloseWindow(SWindow* const pWindow) override;

		virtual void ShowWindow(SWindow* const pWindow) override;
		virtual void HideWindow(SWindow* const pWindow) override;

		virtual void ResizeWindow(const SRect& rect, SWindow* const pWindow) override;
		virtual void ResizeWindow(UInt32 width, UInt32 height, UInt32 center, SWindow* const pWindow) override;

		virtual void ToggleFullSrceen(SWindow* const pWindow) override;
		virtual void Maximized(SWindow* const pWindow) override;
		virtual void Minimized(SWindow* const pWindow) override;
	};

}