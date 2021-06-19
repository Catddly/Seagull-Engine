#pragma once

#include "Common/Core/Defs.h"

#include "Common/Platform/OsDevices.h"

#include "Common/Stl/vector.h"
#include "Common/Math/Vector.h"

namespace SG
{

	class Window;
	class CDeviceManager
	{
	public:
		//! When initializing, collect all the device information once.
		void OnInit();
		void OnShutdown();

		//! Get the information of monitor by index.
		Monitor*  GetMonitor(UInt32 index);
		//! Get the primary monitor.
		Monitor*  GetPrimaryMonitor();
		//! Get current monitor's dpi scale
		Vector2f  GetDpiScale() const;
	private:
		//! Collect all the informations of the monitors and the adapters currently active.
		void      CollectInfos();
	private:
		vector<Monitor>  mMonitors;
		vector<Adapter> mAdapters;
	};

}