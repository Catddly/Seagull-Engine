#pragma once
#include "Common/Config.h"

#include "Common/Base/BasicTypes.h"

namespace SG
{

	// Get the time stamp of current platform
	// For platform abstraction usage

	//enum class ETimeType
	//{
	//	eYear,
	//	eMonth,
	//	eDay,
	//	eHour,
	//	eMinute,
	//	eSecond,
	//};
	
	//! Get low-res system time in hour
	SG_COMMON_API Size GetSystemTimeHour();
	//! Get low-res system time in minute
	SG_COMMON_API Size GetSystemTimeMinute();
	//! Get low-res system time in second
	SG_COMMON_API Size GetSystemTimeSecond();
	//! Get the year of the system
	SG_COMMON_API Size GetSystemTimeYear();
	//! Get the month of the system
	SG_COMMON_API Size GetSystemTimeMonth();
	//! Get the day of the system
	SG_COMMON_API Size GetSystemTimeDay();

}