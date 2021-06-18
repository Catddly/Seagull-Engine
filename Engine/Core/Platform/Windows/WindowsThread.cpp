#include "StdAfx.h"

#include "Common/Thread/IThread.h"
#include "Common/Memory/IMemory.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace SG
{
#ifdef SG_PLATFORM_WINDOWS

	//! Use to forward thread's job.
	static DWORD WINAPI _ThreadFuncForward(void* pUser)
	{
		SThread* pThread = reinterpret_cast<SThread*>(pUser);
		pThread->pFunc(pThread->pUser);
		return 0;
	}

	static char* _CurrThreadName()
	{
		__declspec(thread) static char threadName[64] = "NULL";
		return threadName;
	}

	const char* GetCurrThreadName() 
	{
		return _CurrThreadName();
	}

	void SetCurrThreadName(const char* name)
	{
		strcpy_s(_CurrThreadName(), 64, name);
	}

	bool CreThread(SThread* pThread, ThreadFunc func, void* pUser)
	{
		HANDLE pHandle = ::CreateThread(0, 0, _ThreadFuncForward, pThread, 0, 0);
		if (!pHandle)
			return false;
		else
		{
			pThread->pUser = pUser;
			pThread->pHandle = pHandle;
			pThread->pFunc = func;
			GetThreadID(pThread);
			return true;
		}
	}

	void RestoreThread(SThread* pThread)
	{
		if (pThread->pHandle)
		{
			::WaitForSingleObject(pThread->pHandle, INFINITE);
			::CloseHandle(pThread->pHandle);
			pThread->pHandle = nullptr;
		}
	}

	void SusThread(SThread* pThread)
	{
		::SuspendThread((HANDLE)pThread->pHandle);
	}

	void JoinThread(SThread* pThread)
	{
		::WaitForSingleObject((HANDLE)pThread->pHandle, INFINITE);
	}

	UInt32 GetNumCPUCores() 
	{
		_SYSTEM_INFO sysInfo = {};
		::GetSystemInfo(&sysInfo);
		return (UInt32)sysInfo.dwNumberOfProcessors;
	}
	
	void ThreadSleep(UInt32 ms) 
	{
		::Sleep((DWORD)ms);
	}

	bool GetThreadID(SThread* pThread)
	{
		if (pThread)
		{
			pThread->id = ::GetThreadId((HANDLE)pThread->pHandle);
			return true;
		}
		return false;
	}

	UInt32 GetCurrThreadID()
	{
		return (UInt32)::GetCurrentThreadId();
	}

#endif // SG_PLATFORM_WINDOWS
}