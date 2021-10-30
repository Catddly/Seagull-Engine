#include "StdAfx.h"
#include "Profile/FpsTimer.h"

#include "System/Logger.h"

#include <eastl/algorithm.h>

namespace SG
{

	FpsTimer::FpsTimer(const string& description, float displayInterval, UInt32 averageFrameCnt)
		:mDisplayInterval(displayInterval), mAverageFrameCnt(averageFrameCnt), mCurrIndex(0), mDescription(description)
	{
		mFrameDurationCounter.resize(mAverageFrameCnt);
		for (auto& e : mFrameDurationCounter)
			e = 0.0f;
	}

	void FpsTimer::LogToConsole(const string& info) const
	{
		UInt32 averageFPS = 0;
		UInt32 zeroCounter = 0;

		for (auto& e : mFrameDurationCounter)
		{
			if (e - 0.0f < 0.0000001f) // if e is 0.0f
				++zeroCounter;
			else
				averageFPS += UInt32(1000.0f / e);
		}
		averageFPS /= mAverageFrameCnt - zeroCounter;

		SG_LOG_INFO("%s : %u FPS", info.c_str(), averageFPS);
	}

	void FpsTimer::BeginProfile()
	{
		Tick();
	}

	void FpsTimer::EndProfile()
	{
		Tick();
		mFrameDurationCounter[mCurrIndex] = mPreviousDuration; // record last frame's duration
		mCurrIndex = (mCurrIndex + 1) % mAverageFrameCnt;      // ring buffer

		static Timer loggerTimer;
		if (loggerTimer.GetLifeDurationMs() >= mDisplayInterval * 1000.0f)
		{
			LogToConsole(mDescription);
			loggerTimer.Reset();
		}
		loggerTimer.Tick();
	}

}