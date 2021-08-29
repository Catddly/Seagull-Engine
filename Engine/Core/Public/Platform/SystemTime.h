#pragma once
#include "Core/Config.h"

#include "Base/BasicTypes.h"

namespace SG
{

	// Get the time stamp of current platform
	// For platform abstraction usage

	//! Get low-res system time in hour
	SG_CORE_API Size GetSystemTimeHour();
	//! Get low-res system time in minute
	SG_CORE_API Size GetSystemTimeMinute();
	//! Get low-res system time in second
	SG_CORE_API Size GetSystemTimeSecond();
	//! Get the year of the system
	SG_CORE_API Size GetSystemTimeYear();
	//! Get the month of the system
	SG_CORE_API Size GetSystemTimeMonth();
	//! Get the day of the system
	SG_CORE_API Size GetSystemTimeDay();

}