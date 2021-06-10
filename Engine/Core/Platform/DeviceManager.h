#pragma once

#include "Common/Platform/IDeviceManager.h"

namespace SG
{

	class CDeviceManager : public IDeviceManager
	{
	public:
		virtual void OnInit() override;
		virtual void OnShutdown() override;

		virtual void      CollectInfos() override;
		virtual SMonitor* GetCurrWindowMonitor(SWindow* const pWindow) override;
		virtual SMonitor* GetMonitor(UInt32 index) override;
		virtual SMonitor* GetPrimaryMonitor() override;
	private:
		vector<SAdapter> mAdapters;
		vector<SMonitor> mMonitors;
	};

}