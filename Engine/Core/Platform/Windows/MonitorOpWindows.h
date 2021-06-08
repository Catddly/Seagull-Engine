#pragma once

#include "Common/Platform/IOperatingSystem.h"

namespace SG
{

	struct CWindowsMonitorOp : public IMonitorOp
	{
		virtual int  GetNumMonitors() const override;
		virtual void CollectMonitorInfo(SMonitor* const pMonitor) override;

		virtual Vec2 GetDpiScale() const override;
		virtual void GetCurrentWindowMonitor(SMonitor* const pMonitor, SWindow* const pWindow) override;
	};

}