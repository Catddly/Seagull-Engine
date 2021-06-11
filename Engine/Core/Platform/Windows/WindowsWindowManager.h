#pragma once

#include "Common/Platform/IWindowManager.h"
#include "Common/Platform/IDeviceManager.h"

namespace SG
{

	class CWindowsWindowManager : public IWindowManager
	{
	public:
		virtual void OnInit(SMonitor* const pMonitor) override;
		virtual void OnShutdown() override;

		virtual void OpenWindow(const WChar* windowName, SWindow* const pWindow) override;
		virtual void CloseWindow(SWindow* const pWindow) override;

		virtual void ShowWindow(SWindow* const pWindow) override;
		virtual void HideWindow(SWindow* const pWindow) override;

		virtual void ResizeWindow(const SRect& rect, SWindow* const pWindow) override;
		virtual void ResizeWindow(UInt32 width, UInt32 height, UInt32 center, SWindow* const pWindow) override;

		virtual void ToggleFullSrceen(SWindow* const pWindow) override;
		virtual void Maximized(SWindow* const pWindow) override;
		virtual void Minimized(SWindow* const pWindow) override;

		virtual Vec2 GetMousePosAbsolute() const override;
		virtual Vec2 GetMousePosRelative(SWindow* const pWindow) const override;
	private:
		void AdjustWindow(SWindow* const pWindow, SMonitor* const pMonitor);
		SRect GetRecommandedWindowRect(SMonitor* const pMonitor);
	private:
		SMonitor* mpCurrMonitor = nullptr;
	};

}