#pragma once

#include "Base/BasicTypes.h"
#include "Profile/Timer.h"

#include "Stl/vector.h"
#include "Stl/string.h"

namespace SG
{

	class FpsTimer final : protected Timer
	{
	public:
		SG_CORE_API FpsTimer(const string& description, float displayInterval = 1.0f, UInt32 averageFrameCnt = 24);
		SG_CORE_API ~FpsTimer() = default;

		SG_CORE_API void SetDisplayInterval(float interval) { mDisplayInterval = interval; }

		SG_CORE_API void BeginProfile();
		SG_CORE_API void EndProfile();
	private:
		void LogToConsole(const string& info) const;
	private:
		string mDescription;
		float  mDisplayInterval;
		UInt32 mAverageFrameCnt;

		vector<float> mFrameDurationCounter;
		UInt32        mCurrIndex;
		//float         mDisplayCounter;
	};

}