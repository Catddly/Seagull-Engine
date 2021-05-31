#pragma once
#include "Core/Config.h"

#include "Common/System/ILog.h"
#include "Formatter.h"

namespace SG
{

	class SG_CORE_API CLog final : public ILog
	{
	public:
		CLog() = default;
		~CLog() = default;

		//! Log to console
		virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) override;
		//! Set log format
		virtual void SetFormat(string_view format) override { mFormatter.SetFormat(format); }
	private:
		//! Log information prefix
		void PrintPrefix();
	private:
		enum { SG_LOG_BUFFER_SIZE = 2048 };

		static char sBuffer[SG_LOG_BUFFER_SIZE];
		fmt::CFormatter mFormatter;
	};

}