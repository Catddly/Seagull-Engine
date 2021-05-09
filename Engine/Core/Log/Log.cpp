#include "StdAfx.h"

#include "Common/Base/BasicTypes.h"
#include "Log.h"

namespace SG
{

	char CLog::sBuffer[SG_LOG_BUFFER_SIZE] = { 0 };

	void CLog::LogToConsole(ELogLevel logLevel, const char* format, ...) const
	{
		UInt32 offset = 0;

		va_list args;
		va_start(args, format);
		offset += vsnprintf(sBuffer + offset, SG_LOG_BUFFER_SIZE - offset, format, args);
		va_end(args);

		// end of the log stream buffer
		sBuffer[offset] = '\n';
		sBuffer[offset + 1] = 0;

		PrintUnicode(logLevel, sBuffer);
	}

	void CLog::PrintUnicode(ELogLevel logLevel, const char* buf) const
	{
		bool isError = SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLOG_LEVEL_ERROR | ELogLevel::eLOG_LEVEL_CRITICLE);
		FILE* out = isError ? stderr : stdout;
		// for changing the console color
		HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (isError)
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
			fprintf(out, "%s", buf);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLOG_LEVEL_INFO))
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			fprintf(out, "%s", buf);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLOG_LEVEL_DEBUG))
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
			fprintf(out, "%s", buf);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLOG_LEVEL_WARN))
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			fprintf(out, "%s", buf);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
	}

	void CLog::PrintFormat() const
	{
		printf("%s\n", fmt::formatter::format().c_str());
	}

}