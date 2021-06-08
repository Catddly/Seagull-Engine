#pragma once

#include "Common/Platform/IOperatingSystem.h"

namespace SG
{
	
	class COperatingSystem : public IOperatingSystem
	{
	public:
		virtual void OnInit() override;
		virtual void OnShutdown() override;

	private:
		SWindow          mMainWindow;
		vector<SMonitor> mMonitors;
	};

}