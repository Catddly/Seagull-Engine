#pragma once

#include "Common/Config.h"

namespace SG
{

	// the same as winnt.h, it is just a block of memory
	struct CriticalSection
	{
		void* debugInfo;
		long  lockCount;
		long  recursionCount;
		void* owningThread;
		void* lockSemaphore;
		unsigned long* spinCount;
	};

	// the same as winnt.h, it is just a block of memory
	struct Cv
	{
		void* ptr;
	};

	//! Mutex implemented by CriticalSection
	class SG_COMMON_API Mutex
	{
	public:
		Mutex();
		~Mutex();

		void Lock();
		bool TryLock();
		void UnLock();
	private:
		friend class ConditionVariable;
	private:
		CriticalSection mHandle;
	};

	class SG_COMMON_API ConditionVariable
	{
	public:
		ConditionVariable() = default;
		SG_CLASS_NO_COPY_ASSIGNABLE(ConditionVariable);

		// TODO: replace to UINT32_MAX
		enum { SG_MAX_WAIT_TIME = 0xffffffffui32 };

		bool Wait(const Mutex& mutex, UInt32 ms = SG_MAX_WAIT_TIME);

		void NotifyOne();
		void NotifyAll();
	private:
		Cv   mHandle;
	};

	class SG_COMMON_API Semaphore
	{
	public:
		Semaphore(int maximunCnt, int initCnt = 0);
		~Semaphore();

		void Acquire();
		void Release();
	private:
		void* mHandle;
		volatile int mCount;
	};

}