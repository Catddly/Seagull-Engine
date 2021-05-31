#pragma once

#include "Common/Base/BasicTypes.h"
#include "Common/Base/INoInstance.h"
#include "Common/Platform/ISystemTime.h"

#include "Core/Stl/type_traits.h"
#include "Core/Stl/string.h"

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
				auto str = to_string(GetSystemTimeYear());
				return SG::move(str);
			}
		};
		//! Month of the system (%o)
		struct STimeMonthFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = SG::to_string(GetSystemTimeMonth());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};
		//! Day of the system (%d)
		struct STimeDayFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = SG::to_string(GetSystemTimeDay());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};
		//! Hour in 24-hour system (%h)
		struct STimeHourFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = SG::to_string(GetSystemTimeHour());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};
		//! Minute in 60-minute system (%m)
		struct STimeMinuteFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = SG::to_string(GetSystemTimeMinute());
				if (str.size() == 1)
					str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};
		//! Second in 60-second system (%s)
		struct STimeSecondFormat : public STimeFormat
		{
			static string GetDescription()
			{
				auto str = SG::to_string(GetSystemTimeSecond());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};

		struct SThreadFormat : public SBaseFormat
		{
			string threadName;
			Size   threadId;
			static string GetDescription()
			{
				return "!Thread!";
			}
		};

		template <typename T>
		string format_type()
		{
			SG_COMPILE_ASSERT((SG::is_base_of<SBaseFormat, T>::value));
			return SG::move(T::GetDescription());
		}
	}
}