#pragma once

#include "Common/Base/BasicTypes.h"
#include "Common/Base/INoInstance.h"
#include "Common/Platform/ISystemTime.h"

#include "Core/STL/type_traits.h"
// TODO: use SG::string, remove it
#include <string>

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
				return SG::move(str);
			}
		};
		//! Month of the system (%o)
		struct STimeMonthFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeMonth());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};
		//! Day of the system (%d)
		struct STimeDayFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeDay());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};
		//! Hour in 24-hour system (%h)
		struct STimeHourFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeHour());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};
		//! Minute in 60-minute system (%m)
		struct STimeMinuteFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeMinute());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
			}
		};
		//! Second in 60-second system (%s)
		struct STimeSecondFormat : public STimeFormat
		{
			static std::string GetDescription()
			{
				auto str = std::to_string(GetSystemTimeSecond());
				str = str.size() == 1 ? '0' + str : str;
				return SG::move(str);
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
			return SG::move(T::GetDescription());
		}
	}
}