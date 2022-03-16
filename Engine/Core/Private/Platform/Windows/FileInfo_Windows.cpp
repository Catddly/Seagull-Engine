#include "StdAfx.h"
#include "Core/Private/Platform/Windows/FileInfo_Windows.h"

#include "System/Logger.h"

#include <windows.h>

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

	TimePoint GetFileCreateTime(const char* filename)
	{
		WIN32_FIND_DATAA fd = {};
		if (::FindFirstFileA(filename, &fd) == INVALID_HANDLE_VALUE) // failed to find this file
		{
			SG_LOG_ERROR("There is no file named: %s", filename);
			return TimePoint();
		}

		FILETIME localFileTime = {};
		SYSTEMTIME time = {};
		::FileTimeToLocalFileTime(&fd.ftCreationTime, &localFileTime);
		::FileTimeToSystemTime(&localFileTime, &time);

		TimePoint tp;
		tp.year = time.wYear;
		tp.month = time.wMonth;
		tp.day = time.wDay;
		tp.hour = time.wHour;
		tp.minute = time.wMinute;
		tp.second = time.wSecond;
		return eastl::move(tp);
	}

	TimePoint GetFileLastWriteTime(const char* filename)
	{
		WIN32_FIND_DATAA fd = {};
		if (::FindFirstFileA(filename, &fd) == INVALID_HANDLE_VALUE) // failed to find this file
		{
			SG_LOG_ERROR("There is no file named: %s", filename);
			return TimePoint();
		}

		FILETIME localFileTime = {};
		SYSTEMTIME time = {};
		::FileTimeToLocalFileTime(&fd.ftLastWriteTime, &localFileTime);
		::FileTimeToSystemTime(&localFileTime, &time);

		TimePoint tp;
		tp.year = time.wYear;
		tp.month = time.wMonth;
		tp.day = time.wDay;
		tp.hour = time.wHour;
		tp.minute = time.wMinute;
		tp.second = time.wSecond;
		return eastl::move(tp);
	}

	TimePoint GetFileLastReadTime(const char* filename)
	{
		WIN32_FIND_DATAA fd = {};
		if (::FindFirstFileA(filename, &fd) == INVALID_HANDLE_VALUE) // failed to find this file
		{
			SG_LOG_ERROR("There is no file named: %s", filename);
			return TimePoint();
		}

		FILETIME localFileTime = {};
		SYSTEMTIME time = {};
		::FileTimeToLocalFileTime(&fd.ftLastAccessTime, &localFileTime);
		::FileTimeToSystemTime(&localFileTime, &time);

		TimePoint tp;
		tp.year = time.wYear;
		tp.month = time.wMonth;
		tp.day = time.wDay;
		tp.hour = time.wHour;
		tp.minute = time.wMinute;
		tp.second = time.wSecond;
		return eastl::move(tp);
	}

	void TraverseAllFile(const char* folder, FileTraverseFunc func)
	{
		WIN32_FIND_DATAA fd = {};
		auto hFind = ::FindFirstFileA(folder, &fd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			SG_LOG_WARN("No files in folder: %s", folder);
			::FindClose(hFind);
			return;
		}

		do
		{
			if (!(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0))
				func(fd.cFileName);
		} while (::FindNextFileA(hFind, &fd));
		::FindClose(hFind);
	}

}
#endif