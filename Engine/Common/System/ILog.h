#pragma once
#include "../Config.h"

#include "Common/Base/BasicTypes.h"
#include "Common/System/ISystem.h"

#include <string_view>

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
		//! Log to console
		virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) const = 0;
		//! Set log format
		virtual void SetFormat(std::string_view format) = 0;
		//! Test formatter
		virtual void PrintFormat() const = 0;

		// TODO: After file system
		//void LogToFile(ELogLevel logLevel, ...) const;

		// TODO: After implemented thread, we need to give the logger a mutex
		// Mutex mLogMutex;
	};

#define SG_LOG_INFO(...)  SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_INFO,     __VA_ARGS__);
#define SG_LOG_DEBUG(...) SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_DEBUG,    __VA_ARGS__);
#define SG_LOG_WARN(...)  SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_WARN,     __VA_ARGS__);
#define SG_LOG_ERROR(...) SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_ERROR,    __VA_ARGS__);
#define SG_LOG_CRIT(...)  SG::gModules.pLog->LogToConsole(SG::ELogLevel::eLOG_LEVEL_CRITICLE, __VA_ARGS__);

}