#pragma once

#include "IOperatingSystem.h"

#include "Core/Stl/vector.h"
#include "Core/Stl/string.h"

namespace SG
{

	//! Resolution in pixel.
	typedef struct SResolution
	{
		UInt32 width;
		UInt32 height;

		bool operator==(const SResolution& rhs)
		{
			return width == rhs.width && height == rhs.height;
		}
		bool operator!=(const SResolution& rhs)
		{
			return !operator==(rhs);
		}
	} SResolution;

	//! Monitor information.
	typedef struct SMonitor
	{
		vector<SResolution> resolutions;
		SResolution         defaultResolution;
		SRect               monitorRect;
		SRect               workRect;
		wstring             name;
		wstring             adapterName;
		UInt32              index;
		bool                bIsPrimary;
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