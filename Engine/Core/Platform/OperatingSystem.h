#pragma once
#include "Common/Config.h"

#include "Common/Platform/IOperatingSystem.h"

#include "Common/Platform/DeviceManager.h"
#include "Common/Platform/WindowManager.h"

namespace SG
{
	
	class COperatingSystem : public IOperatingSystem
	{
	public:
		SG_CORE_API virtual void OnInit() override;
		SG_CORE_API virtual void OnShutdown() override;

		SG_CORE_API virtual Monitor* GetMainMonitor() override;
		SG_CORE_API virtual UInt32   GetAdapterCount() override;
		SG_CORE_API virtual Adapter* GetPrimaryAdapter() override;
		SG_CORE_API virtual Window*  GetMainWindow() override;

	private:
		CDeviceManager mDeviceManager;
		CWindowManager mWindowManager;
	};

}