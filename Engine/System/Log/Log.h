#pragma once
#include "Core/Config.h"

#include "Common/System/ILog.h"
#include "Formatter.h"

namespace SG
{

	class CLog final : public ILog
	{
	public:
		CLog() = default;
		~CLog() = default;

		//! Log to console
		SG_CORE_API virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) override;
		//! Set log format
		SG_CORE_API virtual void SetFormat(eastl::string_view format) override { mFormatter.SetFormat(format); }
	private:
		//! Log information prefix
		void PrintPrefix();
	private:
		enum { SG_LOG_BUFFER_SIZE = 2048 };

		static char sBuffer[SG_LOG_BUFFER_SIZE];
		fmt::CFormatter mFormatter;
	};

}