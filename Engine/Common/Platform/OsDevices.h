#pragma once

#include "Common/Core/Defs.h"
#include "IOperatingSystem.h"

#include "Common/Stl/vector.h"
#include "Common/Stl/string.h"

namespace SG
{

	/// Physical res and Logical res.
	/// Physical res is the actual resolution, logical res will be affected
	/// by the scalar of the monitor. 
	/// (Right click on the desktop->Display Setting->Zoom and Layout, you can see the scalar)

	// forward declaration
	class CDeviceManager;

	//! Abstraction of monitor of current PC.
	//! Unable to modify the data by user.
	class Monitor
	{
	public:
		Monitor() = default;
		~Monitor() = default;

		wstring     GetName() const;
		wstring     GetAdapterName() const;
		UInt32      GetIndex() const;
		Rect        GetMonitorRect() const;
		Rect        GetWorkRect() const;
		Resolution  GetDefaultResolution() const;

		bool        IsPrimary() const;
	private:
		friend class CDeviceManager;
	private:
		wstring             mName;              //!< Name of the monitor.
		wstring             mAdapterName;       //!< Adapter name of the monitor.
		UInt32              mIndex;             //!< Index of all the monitors.
		Rect               mMonitorRect;       //!< Window Rect of the monitor. (in logical resolution).
		Rect               mWorkRect;	        //!< Work rect of the monitor.
		vector<Resolution> mResolutions;       //!< All the supported resolution.
		Resolution         mDefaultResolution; //!< Physical resolution.
		bool                bIsPrimary;         //!< Is this monitor a primary monitor.
	};

	//! Abstraction of a adapter of a GPU of kernel display device.
	//! Unable to modify the data by user.
	class Adapter
	{
	public:
		Adapter() = default;
		~Adapter() = default;

		wstring GetName() const;
		wstring GetDisplayName() const;

		bool    IsActive() const;
	private:
		friend class CDeviceManager;
	private:
		wstring          mName;
		wstring          mDisplayName;
		vector<Monitor*> mMonitors;
		bool             bIsActive;
	};

}