#pragma once
#include "Common/Config.h"

#include "Common/Base/BasicTypes.h"
#include "Common/System/ISystem.h"
#include "Common/Base/IIterable.h"

#include <string>
#include "Core/STL/string_view.h"

namespace SG
{

	enum class ELogLevel : UInt32
	{
		eLOG_LEVEL_INFO     = 0x01,
		eLOG_LEVEL_DEBUG    = 0x02,
		eLOG_LEVEL_WARN     = 0x04,
		eLOG_LEVEL_ERROR    = 0x08,
		eLOG_LEVEL_CRITICLE = 0x10
	};
	SG_ENUM_CLASS_FLAG(UInt32, ELogLevel);

	//! @Interface
	//! Abstraction of the logger
	struct SG_COMMON_API ILog
	{
		//! Log to console with printf-like format
		virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) const = 0;
		//! Set log format
		virtual void SetFormat(string_view format) = 0;

		// TODO: After file system
		//void LogToFile(ELogLevel logLevel, ...) const;

		// TODO: After implemented thread, we need to give the logger a mutex
		// Mutex mLogMutex;
	};

namespace impl
{

	template<class T>
	static std::string ParseIterable(IIterable<T>* v, bool reverse)
	{
		std::string s = "";
		s += "[";
		if (!reverse)
		{
			for (auto beg = v->begin(); beg != (v->end() - 1); ++beg)
			{
				s += std::to_string(*beg);
				s += ", ";
			}
			s += std::to_string(*(v->end() - 1));
		}
		else
		{
			for (auto beg = v->rbegin(); beg != (v->rend() - 1); ++beg)
			{
				s += std::to_string(*beg);
				s += ", ";
			}
			s += std::to_string(*(v->rend() - 1));
		}
		s += "]";
		return SG::move(s);
	}

}

#define SG_STR(x) SG_STR_IMPL(x)
#define SG_STR_IMPL(x) #x

#define SG_LOG_INFO(...)  SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_INFO,     __VA_ARGS__);
#define SG_LOG_DEBUG(...) SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_DEBUG,    __VA_ARGS__);
#define SG_LOG_WARN(...)  SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_WARN,     __VA_ARGS__);
#define SG_LOG_ERROR(...) SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_ERROR,    __VA_ARGS__);
#define SG_LOG_CRIT(...)  SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_CRITICLE, __VA_ARGS__);

#define SG_LOG_ITERABLE(LEVEL, ITERABLE)   SG::gModules.pLog->LogToConsole(LEVEL, impl::ParseIterable(&ITERABLE, false).c_str());
#define SG_LOG_ITERABLE_R(LEVEL, ITERABLE) SG::gModules.pLog->LogToConsole(LEVEL, impl::ParseIterable(&ITERABLE, true).c_str());

}