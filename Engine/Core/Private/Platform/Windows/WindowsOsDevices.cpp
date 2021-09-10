#include "StdAfx.h"
#include "Platform/OsDevices.h"

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

	///////////////////////////////////////////////////////////////////////////
	/// Monitor
	///////////////////////////////////////////////////////////////////////////

	SG::wstring Monitor::GetName() const
	{
		return mName;
	}

	SG::wstring Monitor::GetAdapterName() const
	{
		return mAdapterName;
	}

	SG::UInt32 Monitor::GetIndex() const
	{
		return mIndex;
	}

	SG::Rect Monitor::GetMonitorRect() const
	{
		return mMonitorRect;
	}

	SG::Rect Monitor::GetWorkRect() const
	{
		return mWorkRect;
	}

	SG::Resolution Monitor::GetDefaultResolution() const
	{
		return mDefaultResolution;
	}

	bool Monitor::IsPrimary() const
	{
		return bIsPrimary;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Adapter
	///////////////////////////////////////////////////////////////////////////

	SG::wstring Adapter::GetName() const
	{
		return mName;
	}

	SG::wstring Adapter::GetDisplayName() const
	{
		return mDisplayName;
	}

	bool Adapter::IsActive() const
	{
		return bIsActive;
	}

}
#endif // SG_PLATFORM_WINDOWS