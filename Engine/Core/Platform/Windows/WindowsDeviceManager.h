#pragma once

#include "Common/Platform/IDeviceManager.h"
#include "Common/Platform/Window.h"

namespace SG
{

	class CWindowsDeviceManager : public IDeviceManager
	{
	public:
		~CWindowsDeviceManager() = default;

		virtual void OnInit() override;
		virtual void OnShutdown() override;

		virtual void      CollectInfos() override;
		virtual SMonitor* GetMonitor(UInt32 index) override;
		virtual SMonitor* GetPrimaryMonitor() override;
		virtual Vector2f  GetDpiScale() const override;

		static CWindowsDeviceManager* GetInstance();
	protected:
		CWindowsDeviceManager() = default;
	private:
		vector<SAdapter> mAdapters;
		vector<SMonitor> mMonitors;

		static CWindowsDeviceManager* sInstance;
	};

}