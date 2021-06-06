#pragma once
#include "Common/Config.h"

#include "Common/Base/BasicTypes.h"
#include "Common/System/ISystem.h"

#include "Common/System/ISystem.h"

#include <EASTL/string.h>
#include <EASTL/string_view.h>

namespace SG
{

	enum class ELogLevel : UInt32
	{
		eLog_Level_Info     = 0x01,
		eLog_Level_Debug    = 0x02,
		eLog_Level_Warn     = 0x04,
		eLog_Level_Error    = 0x08,
		eLog_Level_Criticle = 0x10
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
	struct SG_COMMON_API ILog
	{
		virtual ~ILog() = default;
		//! Initialize the logger.
		virtual void OnInit() = 0;
		virtual void OnShutdown() = 0;
		//! Set log format.
		virtual void SetFormat(eastl::string_view format) = 0;
		//! Log to console with printf-like format.
		virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) = 0;
		//! Log to file.
		virtual void LogToFile() const = 0;
		//! Set current log mode.
		virtual void SetLogMode(ELogMode logMode) = 0;

		// TODO: After implemented thread, we need to give the logger a mutex
		// Mutex mLogMutex;
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

}

#define SG_STR(x) SG_STR_IMPL(x)
#define SG_STR_IMPL(x) #x

#define SG_LOG_INFO(...)  SG::GetSystemManager()->GetILog()->LogToConsole(SG::ELogLevel::eLog_Level_Info,     __VA_ARGS__);
#define SG_LOG_DEBUG(...) SG::GetSystemManager()->GetILog()->LogToConsole(SG::ELogLevel::eLog_Level_Debug,    __VA_ARGS__);
#define SG_LOG_WARN(...)  SG::GetSystemManager()->GetILog()->LogToConsole(SG::ELogLevel::eLog_Level_Warn,     __VA_ARGS__);
#define SG_LOG_ERROR(...) SG::GetSystemManager()->GetILog()->LogToConsole(SG::ELogLevel::eLog_Level_Error,    __VA_ARGS__);
#define SG_LOG_CRIT(...)  SG::GetSystemManager()->GetILog()->LogToConsole(SG::ELogLevel::eLog_Level_Criticle, __VA_ARGS__);

#define SG_LOG_ITERABLE(LEVEL, BEG, END)   SG::GetSystemManager()->GetILog()->LogToConsole(LEVEL, SG::impl::PrintIterator(BEG, END, false).c_str());
#define SG_LOG_ITERABLE_R(LEVEL, BEG, END) SG::GetSystemManager()->GetILog()->LogToConsole(LEVEL, SG::impl::PrintIterator(BEG, END, true).c_str());

}