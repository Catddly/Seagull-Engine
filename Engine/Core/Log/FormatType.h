#pragma once

#include "Common/Base/BasicTypes.h"
#include "Common/Base/INoInstance.h"
#include "Common/System/Platform/ISystemTime.h"

// TODO: remove it
#include <string>
// TODO: replace to seagull's type_trait
#include <type_traits>

namespace SG
{
	namespace fmt
	{

		//! dummy struct for complete type
		struct SBaseFormat : private INoInstance
		{
			static std::string GetDescription() { return ""; }
		};
		//! Time format
		struct STimeFormat : public SBaseFormat
		{
			static std::string GetDescription() { return ""; }
		};
		//! Year of the system (%y)
		struct STimeYearFormat : public STimeFormat
		{
			static std::string GetDescription() 
			{
				auto str = std::to_string(GetSystemTimeYear());
				return std::move(str);
			}
		};
		//! Month of the system (%o)
		struct STimeMonthFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeMonth());
				str = str.size() == 1 ? '0' + str : str;
				return std::move(str);
			}
		};
		//! Day of the system (%d)
		struct STimeDayFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeDay());
				str = str.size() == 1 ? '0' + str : str;
				return std::move(str);
			}
		};
		//! Hour in 24-hour system (%h)
		struct STimeHourFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeHour());
				str = str.size() == 1 ? '0' + str : str;
				return std::move(str);
			}
		};
		//! Minute in 60-minute system (%m)
		struct STimeMinuteFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeMinute());
				str = str.size() == 1 ? '0' + str : str;
				return std::move(str);
			}
		};
		//! Second in 60-second system (%s)
		struct STimeSecondFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeSecond());
				str = str.size() == 1 ? '0' + str : str;
				return std::move(str);
			}
		};

		struct SThreadFormat : public SBaseFormat
		{
			std::string threadName;
			Size        threadId;
			static std::string GetDescription()
			{
				return "!Thread!";
			}
		};

		template <typename T>
		std::string format_type()
		{
			SG_COMPILE_ASSERT((std::is_base_of_v<SBaseFormat, T>));
			return std::move(T::GetDescription());
		}
	}
}