#include "StdAfx.h"

#include "Common/Core/Defs.h"
#include "Common/Thread/IThread.h"
#include "Common/Memory/IMemory.h"

namespace SG
{
#ifdef SG_PLATFORM_WINDOWS

	//! Use to forward thread's job.
	static DWORD WINAPI _ThreadFuncForward(void* pUser)
	{
		Thread* pThread = reinterpret_cast<Thread*>(pUser);
		pThread->pFunc(pThread->pUser);
		return 0;
	}

	static char* _CurrThreadName()
	{
		SG_THREAD_LOCAL static char threadName[32] = "NULL";
		return threadName;
	}

	const char* GetCurrThreadName() 
	{
		return _CurrThreadName();
	}

	void SetCurrThreadName(const char* name)
	{
		strcpy_s(_CurrThreadName(), 32, name);
	}

	bool ThreadCreate(Thread* pThread, ThreadFunc func, void* pUser)
	{
		HANDLE pHandle = ::CreateThread(0, 0, _ThreadFuncForward, pThread, 0, 0);
		if (!pHandle)
			return false;
		else
		{
			pThread->pUser = pUser;
			pThread->pHandle = pHandle;
			pThread->pFunc = func;
			return true;
		}
	}

	void ThreadRestore(Thread* pThread)
	{
		if (pThread->pHandle)
		{
			::WaitForSingleObject(pThread->pHandle, INFINITE);
			::CloseHandle(pThread->pHandle);
			pThread->pHandle = nullptr;
		}
	}

	void ThreadSuspend(Thread* pThread)
	{
		::SuspendThread((HANDLE)pThread->pHandle);
	}

	void ThreadJoin(Thread* pThread)
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

	UInt32 GetThreadID(Thread* pThread)
	{
		return ::GetThreadId((HANDLE)pThread->pHandle);
	}

	UInt32 GetCurrThreadID()
	{
		return (UInt32)::GetCurrentThreadId();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/// Mutex
	/////////////////////////////////////////////////////////////////////////////////////////

	Mutex::Mutex()
	{
		::InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)&mHandle, 1500);
	}

	Mutex::~Mutex()
	{
		::DeleteCriticalSection((CRITICAL_SECTION*)&mHandle);
	}

	void Mutex::Lock()
	{
		::EnterCriticalSection((CRITICAL_SECTION*)&mHandle);
	}

	bool Mutex::TryLock()
	{
		return ::TryEnterCriticalSection((CRITICAL_SECTION*)&mHandle);
	}

	void Mutex::UnLock()
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)&mHandle);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/// ScopeLock
	/////////////////////////////////////////////////////////////////////////////////////////

	ScopeLock::ScopeLock(Mutex& mutex)
		:mMutex(mutex)
	{
		mMutex.Lock();
	}

	ScopeLock::~ScopeLock()
	{
		mMutex.UnLock();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/// ConditionVariable
	/////////////////////////////////////////////////////////////////////////////////////////

	bool ConditionVariable::Wait(const Mutex& mutex, UInt32 ms)
	{
		return ::SleepConditionVariableCS((CONDITION_VARIABLE*)&mHandle, (CRITICAL_SECTION*)&mutex.mHandle, (DWORD)ms);
	}

	void ConditionVariable::NotifyOne()
	{
		::WakeConditionVariable((CONDITION_VARIABLE*)&mHandle);
	}

	void ConditionVariable::NotifyAll()
	{
		::WakeAllConditionVariable((CONDITION_VARIABLE*)&mHandle);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/// Semaphore
	/////////////////////////////////////////////////////////////////////////////////////////

	Semaphore::Semaphore(int maximunCnt, int initCnt)
		:mCount(initCnt)
	{
		mHandle = (void*)::CreateSemaphore(NULL, initCnt, maximunCnt, NULL);
	}

	Semaphore::~Semaphore()
	{
		::CloseHandle((HANDLE)mHandle);
	}

	void Semaphore::Acquire()
	{
		::WaitForSingleObject((HANDLE)mHandle, INFINITE);
	}

	void Semaphore::Release()
	{
		::ReleaseSemaphore((HANDLE)mHandle, 1, NULL);
	}

#endif // SG_PLATFORM_WINDOWS
}