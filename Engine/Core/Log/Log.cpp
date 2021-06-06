#include "StdAfx.h"

#include "Common/Base/BasicTypes.h"
#include "Log.h"

#include "Core/System/SystemManager.h"

namespace SG
{

	char CLog::sBuffer[SG_MAX_LOG_BUFFER_SIZE] = { 0 };
	UInt32 CLog::sBufferSize = 0;

	void CLog::OnInit()
	{
		ISystemManager* pSysMgr = GetSystemManager();
		IFileSystem* pFs = pSysMgr->GetIFileSystem();
		if (pFs->Open(EResourceDirectory::eLog, "log.txt", EFileMode::eWrite)) // reopen to clean up the log file
		{
			pFs->Close();
		}
	}

	void CLog::LogToConsole(ELogLevel logLevel, const char* format, ...)
	{
		sBufferSize += AddPrefix();
		va_list args;
		va_start(args, format);
		sBufferSize += vsnprintf(sBuffer + sBufferSize, SG_MAX_LOG_BUFFER_SIZE - sBufferSize, format, args);
		va_end(args);

		// end of the log stream buffer
		sBuffer[sBufferSize] = '\n';
		sBuffer[sBufferSize + 1] = 0;
		++sBufferSize;

		bool isError = SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Error | ELogLevel::eLog_Level_Criticle);
		FILE* out = isError ? stderr : stdout;
		HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);

		if (isError)
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
			fprintf(out, "%s", sBuffer);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Info))
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			fprintf(out, "%s", sBuffer);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Debug))
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
			fprintf(out, "%s", sBuffer);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Warn))
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			fprintf(out, "%s", sBuffer);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}

		Flush();
	}

	int CLog::AddPrefix()
	{
		return sprintf_s(sBuffer, SG_MAX_LOG_BUFFER_SIZE, "%s ", mFormatter.GetFormattedString().c_str());
	}

	void CLog::LogToFile() const
	{
		ISystemManager* pSysMgr = GetSystemManager();
		IFileSystem* pFs = pSysMgr->GetIFileSystem();

		if (pFs->Open(EResourceDirectory::eLog, "log.txt", EFileMode::eAppend))
		{
			pFs->Write(sBuffer, sBufferSize);
			pFs->Close();
		}
	}

	void CLog::Flush()
	{
		if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
			LogToFile();
		sBufferSize = 0;
		sBuffer[0] = { 0 };
	}

}