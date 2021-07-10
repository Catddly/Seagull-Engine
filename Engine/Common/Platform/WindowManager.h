#pragma once

#ifdef WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace SG
{

	class Monitor;
	class Window;
	class CWindowManager
	{
	public:
		void OnInit(Monitor* const pMonitor);
		void OnShutdown();

		Window* GetMainWindow() const;
	private:
		static IntPtr __stdcall CustomWinProc(void* hwnd, UInt32 msg, UIntPtr wParam, Int64 lParam);
	private:
		Window* mMainWindow = nullptr;
	};

}