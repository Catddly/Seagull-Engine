#pragma once

#include "Common/Platform/IOperatingSystem.h"
#include "Common/Platform/IDeviceManager.h"
#include "Common/Platform/WindowManager.h"

namespace SG
{
	
	class COperatingSystem : public IOperatingSystem
	{
	public:
		virtual void OnInit() override;
		virtual void OnShutdown() override;
	private:
		IDeviceManager*   mDeviceManager = nullptr;
		WindowManager mWindowManager;
	};

}