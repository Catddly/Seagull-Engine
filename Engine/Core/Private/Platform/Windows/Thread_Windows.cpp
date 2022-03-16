#include "StdAfx.h"

#include "Defs/Defs.h"
#include "Thread/Thread.h"
#include "Memory/Memory.h"

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

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

	Semaphore::InternalSemaphore::InternalSemaphore(int maximunCnt, int initCnt)
	{
		mHandle = (void*)::CreateSemaphore(NULL, initCnt, maximunCnt, NULL);
	}

	Semaphore::InternalSemaphore::~InternalSemaphore()
	{
		::CloseHandle((HANDLE)mHandle);
	}

	void Semaphore::InternalSemaphore::Acquire()
	{
		::WaitForSingleObject((HANDLE)mHandle, INFINITE);
	}

	void Semaphore::InternalSemaphore::Release()
	{
		::ReleaseSemaphore((HANDLE)mHandle, 1, NULL);
	}

	Semaphore::Semaphore(int maximunCnt, int initCnt)
		:mSemaphore(maximunCnt)
	{
		if (initCnt != 0)
			mCount.Exchange(initCnt);
	}

	Semaphore::~Semaphore()
	{
	}

	void Semaphore::Acquire()
	{
		// after decreased mCount, if mCount is below 0, we can acquire the internal semaphore
		if (mCount.Decrease() < 0)
			mSemaphore.Acquire();
	}

	void Semaphore::Release()
	{
		// after increased mCount, if mCount is 0 or below 0, we can release the internal semaphore 
		if (mCount.Increase() <= 0)
			mSemaphore.Release();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/// Atomic16
	/////////////////////////////////////////////////////////////////////////////////////////

	SG::Int16 Atomic16::Increase() volatile
	{
		SG_COMPILE_ASSERT(sizeof(short) == sizeof(Int16), "Unsafe cast from atomic Int16 to short");
		return ::InterlockedIncrement16((volatile short*)&mCount);
	}

	SG::Int16 Atomic16::Decrease() volatile
	{
		SG_COMPILE_ASSERT(sizeof(short) == sizeof(Int16), "Unsafe cast from atomic Int16 to short");
		return ::InterlockedDecrement16((volatile short*)&mCount);
	}

	SG::Int16 Atomic16::Add(Int16 num) volatile
	{
		return ::_InterlockedExchangeAdd16((volatile short*)&mCount, num) + num;
	}

	SG::Int16 Atomic16::Exchange(Int16 num) volatile
	{
		return ::InterlockedExchange16((volatile short*)&mCount, num);
	}

	SG::Int16 Atomic16::Or(Int16 num) volatile
	{
		return ::InterlockedOr16((volatile short*)&mCount, num);
	}

	SG::Int16 Atomic16::And(Int16 num) volatile
	{
		return ::InterlockedOr16((volatile short*)&mCount, num);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/// Atomic32
	/////////////////////////////////////////////////////////////////////////////////////////

	long Atomic32::Increase() volatile
	{
		SG_COMPILE_ASSERT(sizeof(LONG) == sizeof(int), "Unsafe cast from atomic int to LONG");
		return ::InterlockedIncrement((volatile LONG*)&mCount);
	}

	long Atomic32::Decrease() volatile
	{
		SG_COMPILE_ASSERT(sizeof(LONG) == sizeof(int), "Unsafe cast from atomic int to LONG");
		return ::InterlockedDecrement((volatile LONG*)&mCount);
	}

	long Atomic32::Add(long num) volatile
	{
		return ::InterlockedExchangeAdd((volatile LONG*)&mCount, num) + num;
	}

	long Atomic32::Exchange(long num) volatile
	{
		return ::InterlockedExchange((volatile LONG*)&mCount, num);
	}

	long Atomic32::Or(long num) volatile
	{
		return ::InterlockedOr((volatile LONG*)&mCount, num);
	}

	long Atomic32::And(long num) volatile
	{
		return ::InterlockedAnd((volatile LONG*)&mCount, num);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/// Atomic64
	/////////////////////////////////////////////////////////////////////////////////////////

	SG::Int64 Atomic64::Increase() volatile
	{
		SG_COMPILE_ASSERT(sizeof(LONG64) == sizeof(Int64), "Unsafe cast from atomic Int64 to LONG64");
		return ::InterlockedIncrement64((volatile LONG64*)&mCount);
	}

	SG::Int64 Atomic64::Decrease() volatile
	{
		SG_COMPILE_ASSERT(sizeof(LONG64) == sizeof(Int64), "Unsafe cast from atomic Int64 to LONG64");
		return ::InterlockedDecrement64((volatile LONG64*)&mCount);
	}

	SG::Int64 Atomic64::Add(Int64 num) volatile
	{
		return ::InterlockedExchangeAdd64((volatile LONG64*)&mCount, num) + num;
	}

	SG::Int64 Atomic64::Exchange(Int64 num) volatile
	{
		return ::InterlockedExchange64((volatile LONG64*)&mCount, num);
	}

	SG::Int64 Atomic64::Or(Int64 num) volatile
	{
		return ::InterlockedOr64((volatile LONG64*)&mCount, num);
	}

	SG::Int64 Atomic64::And(Int64 num) volatile
	{
		return ::InterlockedOr64((volatile LONG64*)&mCount, num);
	}

}
#endif // SG_PLATFORM_WINDOWS