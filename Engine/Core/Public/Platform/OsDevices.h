#pragma once

#include "Core/Config.h"
#include "Defs/Defs.h"
#include "IOperatingSystem.h"

#include "Stl/vector.h"
#include "Stl/string.h"

namespace SG
{

	/// Physical res and Logical res.
	/// Physical res is the actual resolution, logical res will be affected
	/// by the scalar of the monitor. 
	/// (Right click on the desktop->Display Setting->Zoom and Layout, you can see the scalar)

	// forward declaration
	class DeviceManager;

	//! Abstraction of monitor of current PC.
	//! Unable to modify the data by user.
	class Monitor
	{
	public:
		Monitor() = default;
		~Monitor() = default;

		SG_CORE_API wstring     GetName() const;
		SG_CORE_API wstring     GetAdapterName() const;
		SG_CORE_API UInt32      GetIndex() const;
		SG_CORE_API Rect        GetMonitorRect() const;
		SG_CORE_API Rect        GetWorkRect() const;
		SG_CORE_API Resolution  GetDefaultResolution() const;

		bool        IsPrimary() const;
	private:
		friend class DeviceManager;
	private:
		wstring             mName;              //!< Name of the monitor.
		wstring             mAdapterName;       //!< Adapter name of the monitor.
		UInt32              mIndex;             //!< Index of all the monitors.
		Rect                mMonitorRect;       //!< Window Rect of the monitor. (in logical resolution).
		Rect                mWorkRect;	        //!< Work rect of the monitor.
		vector<Resolution>  mResolutions;       //!< All the supported resolution.
		Resolution          mDefaultResolution; //!< Physical resolution.
		bool                bIsPrimary;         //!< Is this monitor a primary monitor.
	};

	//! Abstraction of a adapter of a GPU of kernel display device.
	//! Unable to modify the data by user.
	class Adapter
	{
	public:
		Adapter() = default;
		~Adapter() = default;

		SG_CORE_API wstring GetName() const;
		SG_CORE_API wstring GetDisplayName() const;

		bool    IsActive() const;
	private:
		friend class DeviceManager;
	private:
		wstring          mName;
		wstring          mDisplayName;
		vector<Monitor*> mMonitors;
		bool             bIsActive;
	};

}