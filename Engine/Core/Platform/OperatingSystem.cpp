#include "StdAfx.h"
#include "OperatingSystem.h"

namespace SG
{

	void COperatingSystem::OnInit()
	{
		mMonitors = CollectMonitorInfos();
		OpenWindow(L"Test", &mMainWindow);
	}

	void COperatingSystem::OnShutdown()
	{

	}

}