#include "StdAfx.h"
#include "WindowsThreadOp.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace SG
{

	//! Use to forward thread's job.
	static DWORD WINAPI _ThreadFuncForward(void* pUser)
	{
		SThread* pThread = reinterpret_cast<SThread*>(pUser);
		pThread->pFunc(pThread->pUser);
		return 0;
	}

	bool SWindowsThreadOp::CreateThread(ThreadFunc func, void* pUser, SThread* pThread)
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

	void SWindowsThreadOp::RestoreThread(SThread* pThread)
	{
	}

	void SWindowsThreadOp::SuspendThread(SThread* pThread)
	{

	}

	void SWindowsThreadOp::JoinThread(SThread* pThread)
	{
		::WaitForSingleObject(pThread->pHandle, INFINITE);
	}

	UInt32 SWindowsThreadOp::GetNumCPUCores() const
	{
		return 0;
	}
	
	void SWindowsThreadOp::Sleep(UInt32 ms) const
	{
	}

	bool SWindowsThreadOp::GetThreadID(SThread* pThread)
	{
		if (pThread)
		{
			pThread->id = ::GetThreadId(pThread->pHandle);
			return true;
		}
		return false;
	}

}