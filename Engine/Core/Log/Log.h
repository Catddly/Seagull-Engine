#pragma once
#include "Core/Config.h"

#include "Common/System/ILog.h"
#include "Formatter.h"

namespace SG
{

	class SG_CORE_API CLog final : public ILog
	{
	public:
		//! Log to console
		virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) const override;
		//! Set log format
		virtual void SetFormat(string_view format) override { fmt::formatter::set_format(format); }
	private:
		//! Log information prefix
		void PrintPrefix() const;
	private:
		enum { SG_LOG_BUFFER_SIZE = 2048 };

		static char sBuffer[SG_LOG_BUFFER_SIZE];
	};

}