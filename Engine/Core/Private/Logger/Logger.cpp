#include "StdAfx.h"
#include "System/Logger.h"

#include "System/FileSystem.h"

namespace SG
{

	char   Logger::sTempBuffer[SG_MAX_TEMP_BUFFER_SIZE] = { 0 };
	int    Logger::sTempBufferSize = 0;
	string Logger::sBuffer;

	ELogMode Logger::mLogMode = ELogMode::eLog_Mode_Default;

	void Logger::OnInit()
	{
		//SetToDefaultFormat();
		SetFormat("[%y:%o:%d]-[%h:%m:%s]");

		if (FileSystem::Open(EResourceDirectory::eLog, "log.txt", EFileMode::efWrite)) // reopen to clean up the log file
		{
			FileSystem::Close();
		}
		sBuffer.resize(SG_MAX_LOG_BUFFER_SIZE);
		sBuffer.clear();
	}

	void Logger::OnShutdown()
	{
		if (sBuffer.length())
		{
			if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
				LogToFile();
		}
	}

	void Logger::LogToConsole(ELogLevel logLevel, const char* format, ...)
	{
		if (mLogMode == ELogMode::eLog_Mode_Quite || mLogMode == ELogMode::eLog_Mode_Quite_No_File)
		{
			if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::efLog_Level_Info | ELogLevel::efLog_Level_Debug))
				return;
		}

		va_list args;
		sTempBufferSize += AddPrefix(sTempBuffer);
		va_start(args, format);
		sTempBufferSize += vsnprintf(sTempBuffer + sTempBufferSize, SG_MAX_LOG_BUFFER_SIZE - sTempBufferSize, format, args);
		va_end(args);

		// end of the log stream buffer
		sTempBuffer[sTempBufferSize] = '\n';
		sTempBuffer[++sTempBufferSize] = { 0 };
		sBuffer.append(sTempBuffer);

		LogOut(logLevel, sTempBuffer);

		if (sBuffer.length() >= SG_MAX_LOG_BUFFER_SIZE - (SG_MAX_SINGLE_LOG_SIZE * 2))
		{
			if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
				LogToFile();
			Flush();
		}
		sTempBufferSize = 0;
		sTempBuffer[0] = { 0 };
	}

	void Logger::LogToConsole(const char* file, int line, ELogLevel logLevel, const char* format, ...)
	{
		if (mLogMode == ELogMode::eLog_Mode_Quite || mLogMode == ELogMode::eLog_Mode_Quite_No_File)
		{
			if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::efLog_Level_Info | ELogLevel::efLog_Level_Debug))
				return;
		}

		va_list args;
		sTempBufferSize += AddPrefix(sTempBuffer, file, line);

		va_start(args, format);
		sTempBufferSize += vsnprintf(sTempBuffer + sTempBufferSize, SG_MAX_LOG_BUFFER_SIZE - sTempBufferSize, format, args);
		va_end(args);

		// end of the log stream buffer
		sTempBuffer[sTempBufferSize] = '\n';
		sTempBuffer[++sTempBufferSize] = { 0 };
		sBuffer.append(sTempBuffer);

		LogOut(logLevel, sTempBuffer);

		if (sBuffer.length() >= SG_MAX_LOG_BUFFER_SIZE - (SG_MAX_SINGLE_LOG_SIZE * 2))
		{
			if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
				LogToFile();
			Flush();
		}
		sTempBufferSize = 0;
		sTempBuffer[0] = { 0 };
	}

	int Logger::AddPrefix(char* pBuf)
	{
		return sprintf_s(pBuf, SG_MAX_TEMP_BUFFER_SIZE, "%s ", fmt::Formatter::GetFormattedString().c_str());
	}

	int Logger::AddPrefix(char* pBuf, const char* file, int line)
	{
		return sprintf_s(pBuf, SG_MAX_TEMP_BUFFER_SIZE, "%s At file: %s, line: %d\nMessage: ", fmt::Formatter::GetFormattedString().c_str(),
			file, line);
	}

	void Logger::LogToFile()
	{
		if (FileSystem::Open(EResourceDirectory::eLog, "log.txt", EFileMode::efAppend))
		{
			FileSystem::Write(sBuffer.data(), sBuffer.length());
			FileSystem::Close();
		}
	}

	void Logger::Flush()
	{
		sBuffer.clear();
	}

	void Logger::LogOut(ELogLevel logLevel, char* pBuffer)
	{
		bool isError = SG_HAS_ENUM_FLAG(logLevel, ELogLevel::efLog_Level_Error | ELogLevel::efLog_Level_Criticle);
		FILE* out = isError ? stderr : stdout;
		HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);

		if (isError)
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
			fprintf(out, "%s", pBuffer);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::efLog_Level_Warn))
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			fprintf(out, "%s", pBuffer);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_No_File)
		{
			if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::efLog_Level_Info))
			{
				::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				fprintf(out, "%s", pBuffer);
				::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			}
			else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::efLog_Level_Debug))
			{
				::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
				fprintf(out, "%s", pBuffer);
				::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			}
		}
	}

}