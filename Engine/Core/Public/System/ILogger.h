#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "System/IModule.h"
#include "Core/Private/System/System.h"

#include <EASTL/string.h>
#include <EASTL/string_view.h>

namespace SG
{

	enum class ELogLevel : UInt32
	{
		efLog_Level_Info     = 0x01,
		efLog_Level_Debug    = 0x02,
		efLog_Level_Warn     = 0x04,
		efLog_Level_Error    = 0x08,
		efLog_Level_Criticle = 0x10
	};
	SG_ENUM_CLASS_FLAG(UInt32, ELogLevel);

	enum class ELogMode
	{
		eLog_Mode_Default = 0,   //! Log all messages and log to file.
		eLog_Mode_No_File,       //! Do not log out as file.
		eLog_Mode_Quite,         //! Only log out the warn, error and criticle message.
		eLog_Mode_Quite_No_File,
	};

	//! @Interface
	//! Abstraction of the logger
	interface ILogger : public IModule
	{
		virtual ~ILogger() = default;

		//! Set the logger format.
		//! %y represent the year.
		//! %o represent the month.
		//! %d represent the day.
		//! %h represent the hour.
		//! %m represent the minute.
		//! %s represent the second.
		//! %t represent the thread name.
		SG_CORE_API virtual void SetFormat(eastl::string_view format) = 0;
		//! Set the logger format to default.
		SG_CORE_API virtual void SetToDefaultFormat() = 0;
		//! Log to console with printf-like format.
		SG_CORE_API virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) = 0;
		//! Log to file.
		SG_CORE_API virtual void LogToFile() const = 0;
		//! Set current log mode.
		SG_CORE_API virtual void SetLogMode(ELogMode logMode) = 0;
	};

namespace impl
{

	template<class T>
	static eastl::string PrintIterator(T* beg, T* end, bool reverse)
	{
		eastl::string str = "[";
		if (!reverse)
		{
			for (; beg != end - 1; beg++)
			{
				str += eastl::to_string(*beg);
				str += " ";
			}
			str += eastl::to_string(*(end - 1));
		}
		else
		{
			--end;
			for (; end != beg; end--)
			{
				str += eastl::to_string(*end);
				str += " ";
			}
			str += eastl::to_string(*(beg));
		}
		str += "]";
		return eastl::move(str);
	}

	template<class T>
	static eastl::string PrintMathTypes(const T& types)
	{
		SG_COMPILE_ASSERT(false, "Please log out a math type!");
		return eastl::string();
	}

	static eastl::string PrintIf(const char* msg, bool val)
	{
		eastl::string temp(msg);
		if (val)
			temp += "Yes";
		else
			temp += "No";
		return eastl::move(temp);
	}

}

#define SG_STR(x) SG_STR_IMPL(x)
#define SG_STR_IMPL(x) #x

#define SG_LOG_INFO(...)  ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(::SG::ELogLevel::efLog_Level_Info,     __VA_ARGS__)
#define SG_LOG_DEBUG(...) ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(::SG::ELogLevel::efLog_Level_Debug,    __VA_ARGS__)
#define SG_LOG_WARN(...)  ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(::SG::ELogLevel::efLog_Level_Warn,     __VA_ARGS__)
#define SG_LOG_ERROR(...) ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(::SG::ELogLevel::efLog_Level_Error,    __VA_ARGS__)
#define SG_LOG_CRIT(...)  ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(::SG::ELogLevel::efLog_Level_Criticle, __VA_ARGS__)

#define SG_LOG_ITERABLE(LEVEL, BEG, END)   ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(LEVEL, ::SG::impl::PrintIterator(BEG, END, false).c_str())
#define SG_LOG_ITERABLE_R(LEVEL, BEG, END) ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(LEVEL, ::SG::impl::PrintIterator(BEG, END, true).c_str())

#define SG_LOG_MATH(LEVEL, VAL)            ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(LEVEL, ::SG::impl::PrintMathTypes(VAL).c_str())

#define SG_LOG_IF(LEVEL, MSG, VAL)         ::SG::CSystem::GetInstance()->GetLogger()->LogToConsole(LEVEL, ::SG::impl::PrintIf(MSG, VAL).c_str())

}