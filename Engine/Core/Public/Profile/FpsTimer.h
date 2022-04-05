#pragma once

#include "Base/BasicTypes.h"
#include "Profile/Timer.h"

#include "Stl/vector.h"
#include "Stl/string.h"

namespace SG
{

	class FpsTimer final : public Timer
	{
	public:
		SG_CORE_API FpsTimer(const string& description, float displayInterval = 1.0f);
		SG_CORE_API ~FpsTimer() = default;

		SG_CORE_API void SetDisplayInterval(float interval) { mDisplayInterval = interval; }

		SG_CORE_API void ProfileScope();
	private:
		void LogToConsole() const;
	private:
		string mDescription;
		float  mDisplayInterval;
	};

}