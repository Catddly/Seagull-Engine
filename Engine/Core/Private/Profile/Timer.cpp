#include "StdAfx.h"
#include "Profile/Timer.h"

namespace SG
{

	Timer::Timer()
	{
		mStartPoint = std::chrono::high_resolution_clock::now();
	}

	void Timer::Tick()
	{
		auto currPoint = std::chrono::high_resolution_clock::now();
		mPreviousDuration = std::chrono::duration<float, std::milli>(currPoint - mStartPoint).count();
		mStartPoint = currPoint;
		mLifeDuration += mPreviousDuration; // accumulate life duration
	}

	void Timer::Reset()
	{
		// reset
		mPreviousDuration = 0.0f;
		mLifeDuration = 0.0f;
		// tick
		mStartPoint = std::chrono::high_resolution_clock::now();
	}

}