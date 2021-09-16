#include "StdAfx.h"

#include "Base/BasicTypes.h"
#include "Logger.h"

#include "System/System.h"

namespace SG
{

	char CLogger::sTempBuffer[SG_MAX_TEMP_BUFFER_SIZE] = { 0 };
	int  CLogger::sTempBufferSize = 0;
	string CLogger::sBuffer;

	void CLogger::OnInit()
	{
		SetToDefaultFormat();

		auto* pSysMgr = SSystem();
		IFileSystem* pFs = pSysMgr->GetFileSystem();
		if (pFs->Open(EResourceDirectory::eLog, "log.txt", EFileMode::efWrite)) // reopen to clean up the log file
		{
			pFs->Close();
		}
		sBuffer.resize(SG_MAX_LOG_BUFFER_SIZE);
		sBuffer.clear();
	}

	void CLogger::OnShutdown()
	{
		if (sBuffer.length())
		{
			if (mLogMode == ELogMode::eLog_Mode_Default || mLogMode == ELogMode::eLog_Mode_Quite)
				LogToFile();
		}
	}

	void CLogger::LogToConsole(ELogLevel logLevel, const char* format, ...)
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

	int CLogger::AddPrefix(char* pBuf)
	{
		return sprintf_s(pBuf, SG_MAX_TEMP_BUFFER_SIZE, "%s ", mFormatter.GetFormattedString().c_str());
	}

	void CLogger::LogToFile() const
	{
		auto* pSysMgr = SSystem();
		IFileSystem* pFs = pSysMgr->GetFileSystem();

		if (pFs->Open(EResourceDirectory::eLog, "log.txt", EFileMode::efAppend))
		{
			pFs->Write(sBuffer.data(), sBuffer.length());
			pFs->Close();
		}
	}

	void CLogger::Flush()
	{
		sBuffer.clear();
	}

	void CLogger::LogOut(ELogLevel logLevel, char* pBuffer)
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