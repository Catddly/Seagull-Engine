#pragma once

#include "Defs/Defs.h"

#include "Platform/OsDevices.h"

#include "Stl/vector.h"
#include "Math/Vector.h"

namespace SG
{

	class Window;
	class DeviceManager
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

		UInt32    GetAdapterCount() const;
		Adapter*  GetPrimaryAdapter();
	private:
		//! Collect all the informations of the monitors and the adapters currently active.
		void      CollectInfos();
	private:
		vector<Monitor> mMonitors;
		vector<Adapter> mAdapters;
		UInt32          mAdapterCount;
	};

}