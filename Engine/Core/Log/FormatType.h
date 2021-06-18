#pragma once

#include "Common/Base/BasicTypes.h"
#include "Common/Base/INoInstance.h"
#include "Common/Platform/ISystemTime.h"
#include "Common/Thread/IThread.h"

#include "Common/Stl/string.h"
#include <EASTL/type_traits.h>

namespace SG
{
	namespace fmt
	{

		//! dummy struct for complete type
		struct SBaseFormat : private INoInstance
		{
			static string GetDescription() { return ""; }
		};
		//! Time format
		struct STimeFormat : public SBaseFormat
		{
			static string GetDescription() { return ""; }
		};
		//! Year of the system (%y)
		struct STimeYearFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeYear());
				return move(str);
			}
		};
		//! Month of the system (%o)
		struct STimeMonthFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeMonth());
				str = str.size() == 1 ? '0' + str : str;
				return move(str);
			}
		};
		//! Day of the system (%d)
		struct STimeDayFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeDay());
				str = str.size() == 1 ? '0' + str : str;
				return move(str);
			}
		};
		//! Hour in 24-hour system (%h)
		struct STimeHourFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeHour());
				str = str.size() == 1 ? '0' + str : str;
				return move(str);
			}
		};
		//! Minute in 60-minute system (%m)
		struct STimeMinuteFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeMinute());
				if (str.size() == 1)
					str = str.size() == 1 ? '0' + str : str;
				return move(str);
			}
		};
		//! Second in 60-second system (%s)
		struct STimeSecondFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeSecond());
				str = str.size() == 1 ? '0' + str : str;
				return move(str);
			}
		};

		struct SThreadFormat : public SBaseFormat
		{
			string threadName;
			Size   threadId;
			static string GetDescription()
			{
				return GetCurrThreadName();
			}
		};

		template <typename T>
		string format_type()
		{
			SG_COMPILE_ASSERT((eastl::is_base_of<SBaseFormat, T>::value), "Format type is not base of SBaseForamt");
			return move(T::GetDescription());
		}
	}
}