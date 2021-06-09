#include "StdAfx.h"
#include "OperatingSystem.h"

namespace SG
{

	void COperatingSystem::OnInit()
	{
		OpenWindow(L"Test", &mMainWindow);

		int numMonitors = GetNumMonitors();
		mMonitors.resize(numMonitors);
		for (int i = 0; i < mMonitors.size(); i++)
			CollectMonitorInfo(i, &mMonitors[i]);
		
		GetWindowMonitorIndex(mMonitors, &mMainWindow);
	}

	void COperatingSystem::OnShutdown()
	{

	}

}