#include "StdAfx.h"
#include "System/Logger.h"

#include "Profile/Profile.h"

namespace SG
{

	char   Logger::sTempBuffer[SG_MAX_TEMP_BUFFER_SIZE] = { 0 };
	int    Logger::sTempBufferSize = 0;
	string Logger::sFileLogOutCache = "";

	ELogMode Logger::mLogMode = ELogMode::eLog_Mode_Default;
	FileStream Logger::mFileStream;

	void Logger::OnInit()
	{
		SG_PROFILE_FUNCTION();

		SetFormat("[%y:%o:%d]-[%h:%m:%s]");

		FileSystem::RemoveFile(EResourceDirectory::eLog, "log.txt");
		FileSystem::SetFileStream(&mFileStream);
		if (!FileSystem::Open(EResourceDirectory::eLog, "log.txt", EFileMode::efWrite))
		{
			printf("Failed to open logger's file stream!\n");
			SG_ASSERT(false);
		}
		FileSystem::SetToDefaultFileStream();

		sFileLogOutCache.resize(SG_MAX_LOG_BUFFER_SIZE);
		sFileLogOutCache.clear();
	}

	void Logger::OnShutdown()
	{
		SG_PROFILE_FUNCTION();

		if (sFileLogOutCache.length())
		{
			if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
				LogToFile();
		}

		FileSystem::SetFileStream(&mFileStream);
		FileSystem::Close();
		FileSystem::SetToDefaultFileStream();
	}

	bool Logger::NeedLogToConsole(ELogLevel logLevel)
	{
		if (mLogMode == ELogMode::eLog_Mode_Quite || mLogMode == ELogMode::eLog_Mode_Quite_No_File)
		{
			if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::efLog_Level_Info | ELogLevel::efLog_Level_Debug))
				return false;
		}
		return true;
	}

	void Logger::ClearTempBuffer()
	{
		sTempBufferSize = 0;
		sTempBuffer[0] = { 0 };
	}

	void Logger::LogToConsole(ELogLevel logLevel, const char* format, ...)
	{
		SG_PROFILE_FUNCTION();

		if (!NeedLogToConsole(logLevel))
			return;

		sTempBufferSize += AddPrefix(sTempBuffer);

		va_list args;
		va_start(args, format);
		sTempBufferSize += vsnprintf(sTempBuffer + sTempBufferSize, SG_MAX_LOG_BUFFER_SIZE - sTempBufferSize, format, args);
		va_end(args);

		// end of the log stream buffer
		sTempBuffer[sTempBufferSize] = '\n';
		sTempBuffer[++sTempBufferSize] = { 0 };
		sFileLogOutCache.append(sTempBuffer);

		LogToConsole(logLevel, sTempBuffer);

		if (sFileLogOutCache.length() >= SG_MAX_TEMP_BUFFER_SIZE)
			FlushToDisk();

		ClearTempBuffer();
	}

	void Logger::LogToConsole(const char* file, int line, ELogLevel logLevel, const char* format, ...)
	{
		SG_PROFILE_FUNCTION();

		if (!NeedLogToConsole(logLevel))
			return;

		sTempBufferSize += AddPrefix(sTempBuffer, file, line);

		va_list args;
		va_start(args, format);
		sTempBufferSize += vsnprintf(sTempBuffer + sTempBufferSize, SG_MAX_LOG_BUFFER_SIZE - sTempBufferSize, format, args);
		va_end(args);

		// end of the log stream buffer
		sTempBuffer[sTempBufferSize] = '\n';
		sTempBuffer[++sTempBufferSize] = { 0 };
		sFileLogOutCache.append(sTempBuffer);

		LogToConsole(logLevel, sTempBuffer);

		if (sFileLogOutCache.length() >= SG_MAX_TEMP_BUFFER_SIZE)
			FlushToDisk();

		ClearTempBuffer();
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
		SG_PROFILE_FUNCTION();

		FileSystem::SetFileStream(&mFileStream);
		FileSystem::Write(sFileLogOutCache.data(), sFileLogOutCache.length());
		FileSystem::SetToDefaultFileStream();
	}

	void Logger::FlushToDisk()
	{
		SG_PROFILE_FUNCTION();

		if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
			LogToFile();
		sFileLogOutCache.clear();
	}

	void Logger::LogToConsole(ELogLevel logLevel, char* pBuffer)
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