#pragma once
#include "Core/Config.h"

#include "System/ILogger.h"
#include "System/IModule.h"
#include "Formatter.h"

#include "Stl/string.h"

#include <stdarg.h>

namespace SG
{

	class CLogger final : public ILogger
	{
	public:
		SG_CORE_API virtual void OnInit() override;
		SG_CORE_API virtual void OnShutdown() override;
		//! Set log format
		SG_CORE_API virtual void SetFormat(string_view format) override { mFormatter.SetFormat(format); }
		SG_CORE_API virtual void SetToDefaultFormat() override { SetFormat("[%y:%o:%d]-[%h:%m:%s]-[%t]"); }
		//! Log to console
		SG_CORE_API virtual void LogToConsole(ELogLevel logLevel, const char* format, ...) override;
		SG_CORE_API virtual void LogToFile() const override;

		SG_CORE_API virtual void SetLogMode(ELogMode logMode) override { mLogMode = logMode; }

		SG_CORE_API virtual const char* GetRegisterName() const override { return "Logger"; }
		//SG_CORE_API virtual Handle GetModuleHandle() override;
	private:
		//! Add log information prefix to buffer.
		//! @param  (pBuf) buffer to add to.
		//! @return offset of the buffer.
		int  AddPrefix(char* pBuf);
		void Flush();
		void LogOut(ELogLevel logLevel, char* pBuffer);
	private:
		enum { SG_MAX_LOG_BUFFER_SIZE = 4096, SG_MAX_TEMP_BUFFER_SIZE = SG_MAX_LOG_BUFFER_SIZE / 4, SG_MAX_SINGLE_LOG_SIZE = 512 };

		static char sTempBuffer[SG_MAX_TEMP_BUFFER_SIZE];
		static int  sTempBufferSize;
		static string sBuffer;

		ELogMode mLogMode = ELogMode::eLog_Mode_Default;
		fmt::CFormatter mFormatter;
	};

}