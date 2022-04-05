#include "StdAfx.h"
#include "Profile/Timer.h"

namespace SG
{

	Timer::Timer()
	{
		mLastTickPoint = std::chrono::high_resolution_clock::now();
		mbTimerStoped = false;
	}

	void Timer::Start()
	{
		Reset();
		mbTimerStoped = false;
	}

	void Timer::Stop()
	{
		mbTimerStoped = true;
	}

	void Timer::Tick()
	{
		auto currPoint = std::chrono::high_resolution_clock::now();
		mPreviousDuration = std::chrono::duration<float, std::milli>(currPoint - mLastTickPoint).count();
		mLastTickPoint = currPoint;
		if (!mbTimerStoped)
			mLifeDuration += mPreviousDuration; // accumulate life duration
	}

	void Timer::Reset()
	{
		// reset
		mPreviousDuration = 0.0f;
		mLifeDuration = 0.0f;
		// tick
		mLastTickPoint = std::chrono::high_resolution_clock::now();
	}

}