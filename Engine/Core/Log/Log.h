#pragma once
#include "Core/Config.h"

#include "Common/System/ILog.h"
#include "Formatter.h"

namespace SG
{

	class CLog final : public ILog
	{
	public:
		SG_CORE_API virtual void OnInit() override;
		//! Set log format
		SG_CORE_API virtual void SetFormat(string_view format) override { mFormatter.SetFormat(format); }
		//! Log to console
		SG_CORE_API virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) override;
		SG_CORE_API virtual void LogToFile() const override;

		SG_CORE_API virtual void SetLogMode(ELogMode logMode) override { mLogMode = logMode; }
	private:
		//! Add log information prefix to buffer.
		//! @return offset of the buffer.
		int  AddPrefix();
		void Flush();
	private:
		enum { SG_MAX_LOG_BUFFER_SIZE = 2048 };

		static char sBuffer[SG_MAX_LOG_BUFFER_SIZE];
		static UInt32 sBufferSize;

		ELogMode mLogMode = ELogMode::eLog_Mode_Default;

		fmt::CFormatter mFormatter;
	};

}