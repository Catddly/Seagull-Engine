#include "StdAfx.h"

#include "Common/Base/BasicTypes.h"
#include "Log.h"

#include "Core/System/SystemManager.h"

namespace SG
{

	char CLog::sTempBuffer[SG_MAX_TEMP_BUFFER_SIZE] = { 0 };
	string CLog::sBuffer;
	int CLog::sBufferSize = 0;

	void CLog::OnInit()
	{
		SetFormat("[%y:%o:%d]-[%h:%m:%s]"); // default format.
		ISystemManager* pSysMgr = CSystemManager::GetInstance();
		IFileSystem* pFs = pSysMgr->GetIFileSystem();
		if (pFs->Open(EResourceDirectory::eLog, "log.txt", EFileMode::eWrite)) // reopen to clean up the log file
		{
			pFs->Close();
		}
		sBuffer.resize(SG_MAX_LOG_BUFFER_SIZE);
		sBuffer.clear();
	}

	void CLog::OnShutdown()
	{
		if (sBuffer.length())
		{
			if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
				LogToFile();
		}
	}

	void CLog::LogToConsole(ELogLevel logLevel, const char* format, ...)
	{
		if (mLogMode == ELogMode::eLog_Mode_Quite || mLogMode == ELogMode::eLog_Mode_Quite_No_File)
		{
			if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Info | ELogLevel::eLog_Level_Debug))
				return;
		}

		va_list args;
		sBufferSize += AddPrefix(sTempBuffer);
		va_start(args, format);
		sBufferSize += vsnprintf(sTempBuffer + sBufferSize, SG_MAX_LOG_BUFFER_SIZE - sBufferSize, format, args);
		va_end(args);

		// end of the log stream buffer
		sTempBuffer[sBufferSize] = '\n';
		sTempBuffer[++sBufferSize] = { 0 };
		sBuffer.append(sTempBuffer);

		bool isError = SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Error | ELogLevel::eLog_Level_Criticle);
		FILE* out = isError ? stderr : stdout;
		HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);

		if (isError)
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
			fprintf(out, "%s", sTempBuffer);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Warn))
		{
			::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			fprintf(out, "%s", sTempBuffer);
			::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_No_File)
		{
			if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Info))
			{
				::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				fprintf(out, "%s", sTempBuffer);
				::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			}
			else if (SG_HAS_ENUM_FLAG(logLevel, ELogLevel::eLog_Level_Debug))
			{
				::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
				fprintf(out, "%s", sTempBuffer);
				::SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			}
		}

		if (sBuffer.length() >= SG_MAX_LOG_BUFFER_SIZE - 150)
		{
			if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
				LogToFile();
			Flush();
		}
		sBufferSize = 0;
		sTempBuffer[0] = { 0 };
	}

	int CLog::AddPrefix(char* pBuf)
	{
		return sprintf_s(pBuf, SG_MAX_TEMP_BUFFER_SIZE, "%s ", mFormatter.GetFormattedString().c_str());
	}

	void CLog::LogToFile() const
	{
		ISystemManager* pSysMgr = CSystemManager::GetInstance();
		IFileSystem* pFs = pSysMgr->GetIFileSystem();

		if (pFs->Open(EResourceDirectory::eLog, "log.txt", EFileMode::eAppend))
		{
			pFs->Write(sBuffer.data(), sBuffer.length());
			pFs->Close();
		}
	}

	void CLog::Flush()
	{
		sBuffer.clear();
	}

}