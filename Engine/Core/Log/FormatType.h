#pragma once

#include "Common/Base/BasicTypes.h"
#include "Common/Base/INoInstance.h"
#include "Common/Platform/ISystemTime.h"

#include <EASTL/string.h>
#include <EASTL/type_traits.h>

namespace SG
{
	namespace fmt
	{

		//! dummy struct for complete type
		struct SBaseFormat : private INoInstance
		{
			static eastl::string GetDescription() { return ""; }
		};
		//! Time format
		struct STimeFormat : public SBaseFormat
		{
			static eastl::string GetDescription() { return ""; }
		};
		//! Year of the system (%y)
		struct STimeYearFormat : public STimeFormat
		{
			static eastl::string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeYear());
				return eastl::move(str);
			}
		};
		//! Month of the system (%o)
		struct STimeMonthFormat : public STimeFormat
		{
			static eastl::string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeMonth());
				str = str.size() == 1 ? '0' + str : str;
				return eastl::move(str);
			}
		};
		//! Day of the system (%d)
		struct STimeDayFormat : public STimeFormat
		{
			static eastl::string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeDay());
				str = str.size() == 1 ? '0' + str : str;
				return eastl::move(str);
			}
		};
		//! Hour in 24-hour system (%h)
		struct STimeHourFormat : public STimeFormat
		{
			static eastl::string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeHour());
				str = str.size() == 1 ? '0' + str : str;
				return eastl::move(str);
			}
		};
		//! Minute in 60-minute system (%m)
		struct STimeMinuteFormat : public STimeFormat
		{
			static eastl::string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeMinute());
				if (str.size() == 1)
					str = str.size() == 1 ? '0' + str : str;
				return eastl::move(str);
			}
		};
		//! Second in 60-second system (%s)
		struct STimeSecondFormat : public STimeFormat
		{
			static eastl::string GetDescription()
			{
				auto str = eastl::to_string(GetSystemTimeSecond());
				str = str.size() == 1 ? '0' + str : str;
				return eastl::move(str);
			}
		};

		struct SThreadFormat : public SBaseFormat
		{
			eastl::string threadName;
			Size   threadId;
			static eastl::string GetDescription()
			{
				return "!Thread!";
			}
		};

		template <typename T>
		eastl::string format_type()
		{
			SG_COMPILE_ASSERT((eastl::is_base_of<SBaseFormat, T>::value));
			return eastl::move(T::GetDescription());
		}
	}
}