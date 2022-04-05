#pragma once

#include "Core/Config.h"

#include <chrono>

namespace SG
{

	//! High-resolution timer.
	class Timer
	{
	public:
		SG_CORE_API Timer();
		SG_CORE_API virtual ~Timer() = default;

		SG_CORE_API void Start();
		SG_CORE_API void Stop();

		//! Tick to record the time last so far.
		//! Record between the timer was created or ticked and next time the timer ticks.
		SG_CORE_API virtual void Tick();

		//! Reset the duration of this timer.
		//! Reset also tick once for you.
		SG_CORE_API void Reset();

		SG_CORE_API inline float GetDurationMs()     const { return mPreviousDuration; }
		SG_CORE_API inline float GetDurationNano()   const { return mPreviousDuration * 1000.0f; }
		SG_CORE_API inline float GetDurationSecond() const { return mPreviousDuration / 1000.0f; }

		SG_CORE_API inline float GetLifeDurationMs() const { return mLifeDuration; };

		SG_CORE_API inline operator float() const  { return mPreviousDuration; }
		SG_CORE_API inline operator double() const { return mPreviousDuration; }
	protected:
		std::chrono::steady_clock::time_point mLastTickPoint;
		float mPreviousDuration;
		float mLifeDuration;
		bool  mbTimerStoped = false;
	};

}