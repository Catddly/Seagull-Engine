#pragma once
//#include "Common/Config.h"

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
	Size GetSystemTimeHour();
	//! Get low-res system time in minute
	Size GetSystemTimeMinute();
	//! Get low-res system time in second
	Size GetSystemTimeSecond();
	//! Get the year of the system
	Size GetSystemTimeYear();
	//! Get the month of the system
	Size GetSystemTimeMonth();
	//! Get the day of the system
	Size GetSystemTimeDay();

}