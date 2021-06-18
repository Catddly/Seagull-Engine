#pragma once

namespace SG
{

	struct SMonitor;
	class Window;
	class WindowManager
	{
	public:
		void OnInit(SMonitor* const pMonitor);
		void OnShutdown();
	private:
		Window* mMainWindow = nullptr;
	};

}