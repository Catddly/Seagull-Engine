#pragma once

#include "Common/Platform/IWindowManager.h"
#include "Common/Platform/IDeviceManager.h"

namespace SG
{

	class CWindowsWindowManager : public IWindowManager
	{
	public:
		~CWindowsWindowManager() = default;

		virtual void OnInit(SMonitor* const pMonitor) override;
		virtual void OnShutdown() override;

		virtual void OpenWindow(const WChar* windowName, SWindow* const pWindow) override;
		virtual void CloseWindow(SWindow* const pWindow) override;

		virtual void ShowWindow(SWindow* const pWindow) override;
		virtual void HideWindow(SWindow* const pWindow) override;

		virtual void ResizeWindow(const SRect& rect, SWindow* const pWindow) override;
		virtual void ResizeWindow(UInt32 width, UInt32 height, SWindow* const pWindow) override;

		virtual void ToggleFullSrceen(SWindow* const pWindow) override;
		virtual void Maximized(SWindow* const pWindow) override;
		virtual void Minimized(SWindow* const pWindow) override;

		virtual Vector2f GetMousePosAbsolute() const override;
		virtual Vector2f GetMousePosRelative(SWindow* const pWindow) const override;

		static CWindowsWindowManager* GetInstance();
	protected:
		CWindowsWindowManager() = default;

		void AdjustWindow(SWindow* const pWindow);
		SRect GetRecommandedWindowRect();
	private:
		SMonitor* mpCurrMonitor = nullptr;

		static CWindowsWindowManager* sInstance;
	};

}