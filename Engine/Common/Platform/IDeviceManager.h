#pragma once

#include "IOperatingSystem.h"

#include "Common/Stl/vector.h"
#include "Common/Stl/string.h"

namespace SG
{

	/// Physical res and Logical res.
	/// Physical res is the actual resolution, logical res will be affected
	/// by the scalar of the monitor. 
	/// (Right click on the desktop->Display Setting->Zoom and Layout, you can see the scalar)

	//! Monitor information.
	typedef struct SMonitor
	{
		vector<SResolution> resolutions;       //! All the supported resolution.
		SResolution         defaultResolution; //! Physical resolution.
		SRect               monitorRect;       //! Window Rect of the monitor. (in logical resolution).
		SRect               workRect;          //! Workd Rect of the monitor.
		wstring             name;              //! Name of the monitor.
		wstring             adapterName;       //! Adapter name of the monitor.
		UInt32              index;             //! Index of all the monitors.
		bool                bIsPrimary;        //! Is this monitor a primary monitor.
	} SMonitor;

	//! Adapter information.
	//! Adapter is a render hardware.
	typedef struct SAdapter
	{
		wstring name;
		wstring adapterName;
		vector<SMonitor*> monitors;
		bool    bIsActive;
	} SAdapter;

	struct SWindow;

	struct IDeviceManager
	{
		virtual ~IDeviceManager() = default;

		//! When initializing, collect all the device information once.
		virtual void OnInit() = 0;
		virtual void OnShutdown() = 0;

		//! Collect all the informations of the monitors and the adapters currently active.
		virtual void      CollectInfos() = 0;
		//! Get the index of the monitor where current window is on.
		virtual SMonitor* GetCurrWindowMonitor(SWindow* const pWindow) = 0;
		//! Get the information of monitor by index.
		virtual SMonitor* GetMonitor(UInt32 index) = 0;
		//! Get the primary monitor.
		virtual SMonitor* GetPrimaryMonitor() = 0;
	};

}