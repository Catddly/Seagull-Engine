#include "StdAfx.h"
#include "Profile/FpsTimer.h"

#include "System/Logger.h"

#include <eastl/algorithm.h>

namespace SG
{

	FpsTimer::FpsTimer(const string& description, float displayInterval)
		:mDisplayInterval(displayInterval), mDescription(description)
	{
	}

	void FpsTimer::LogToConsole() const
	{
		SG_LOG_INFO("%s : %u FPS", mDescription.c_str(), UInt32(1.0f / GetDurationSecond()));
	}

	void FpsTimer::ProfileScope()
	{
		Tick();
	}

}