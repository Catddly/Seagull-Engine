#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"
#include "System/FileSystem.h"

#include "Core/Private/Logger/Formatter.h"

#include "eastl/string.h"
#include "eastl/string_view.h"

namespace SG
{

	enum class ELogMode
	{
		eLog_Mode_Default = 0,   //! Log all messages and log to file.
		eLog_Mode_No_File,       //! Do not log out as file.
		eLog_Mode_Quite,         //! Only log out the warn, error and critical message.
		eLog_Mode_Quite_No_File,
	};

	enum class ELogLevel : UInt32
	{
		efLog_Level_Info     = 0x01,
		efLog_Level_Debug    = 0x02,
		efLog_Level_Warn     = 0x04,
		efLog_Level_Error    = 0x08,
		efLog_Level_Criticle = 0x10
	};
	SG_ENUM_CLASS_FLAG(UInt32, ELogLevel);

	class Logger
	{
	public:
		//! Set log format
		SG_CORE_API static void SetFormat(const char* format) { fmt::Formatter::SetFormat(format); }
		SG_CORE_API static void SetToDefaultFormat() { SetFormat("[%y:%o:%d]-[%h:%m:%s]-[%t]"); }
		//! Log to console
		SG_CORE_API static void LogToConsole(ELogLevel logLevel, const char* format, ...);
		SG_CORE_API static void LogToConsole(const char* file, int line, ELogLevel logLevel, const char* format, ...);
		SG_CORE_API static void LogToFile();

		SG_CORE_API static void FlushToDisk();

		static void SetLogMode(ELogMode logMode) { mLogMode = logMode; }
	private:
		friend class System;

		static void OnInit();
		static void OnShutdown();
	private:
		static bool NeedLogToConsole(ELogLevel logLevel);
		static void ClearTempBuffer();
		//! Add log information prefix to buffer.
		//! @param  (pBuf) buffer to add to.
		//! @return offset of the buffer.
		static int  AddPrefix(char* pBuf);
		static int  AddPrefix(char* pBuf, const char* file, int line);
		static void LogToConsole(ELogLevel logLevel, char* pBuffer);
	private:
		enum { SG_MAX_LOG_BUFFER_SIZE = 1024 * 8, SG_MAX_TEMP_BUFFER_SIZE = SG_MAX_LOG_BUFFER_SIZE / 2, SG_MAX_SINGLE_LOG_SIZE = 2048 };

		//typedef eastl::array<eastl::array<char, SG_MAX_TEMP_BUFFER_SIZE>, 2> DoubleCache;
		//static DoubleCache mLogCaches;

		static char sTempBuffer[SG_MAX_TEMP_BUFFER_SIZE];
		static int  sTempBufferSize;

		static string sFileLogOutCache;

		static ELogMode   mLogMode;
		static FileStream mFileStream;
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
	static eastl::string PrintMathTypes(const T& types, const string& prefix)
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

#define SG_LOG_INFO(...)  ::SG::Logger::LogToConsole(::SG::ELogLevel::efLog_Level_Info,  __VA_ARGS__)
#define SG_LOG_DEBUG(...) ::SG::Logger::LogToConsole(::SG::ELogLevel::efLog_Level_Debug, __VA_ARGS__)
#define SG_LOG_WARN(...)  ::SG::Logger::LogToConsole(__FILE__, __LINE__, ::SG::ELogLevel::efLog_Level_Warn,     __VA_ARGS__)
#define SG_LOG_ERROR(...) ::SG::Logger::LogToConsole(__FILE__, __LINE__, ::SG::ELogLevel::efLog_Level_Error,    __VA_ARGS__)
#define SG_LOG_CRIT(...)  ::SG::Logger::LogToConsole(__FILE__, __LINE__, ::SG::ELogLevel::efLog_Level_Criticle, __VA_ARGS__)

#define SG_LOG_ITERABLE(LEVEL, BEG, END)   ::SG::Logger::LogToConsole(LEVEL, ::SG::impl::PrintIterator(BEG, END, false).c_str())
#define SG_LOG_ITERABLE_R(LEVEL, BEG, END) ::SG::Logger::LogToConsole(LEVEL, ::SG::impl::PrintIterator(BEG, END, true).c_str())

#define SG_LOG_MATH(LEVEL, VAL, PREFIX)    ::SG::Logger::LogToConsole(LEVEL, ::SG::impl::PrintMathTypes(VAL, PREFIX).c_str())

#define SG_LOG_IF(LEVEL, MSG, VAL)         ::SG::Logger::LogToConsole(LEVEL, ::SG::impl::PrintIf(MSG, VAL).c_str())

}