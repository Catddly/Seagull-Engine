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

	class SG_COMMON_API Atomic16
	{
	public:
		Atomic16() 
			: mCount(0)
		{}
		~Atomic16() = default;

		Int16 Increase() volatile;
		Int16 Decrease() volatile;
		Int16 Add(Int16 num) volatile;
		Int16 Exchange(Int16 num) volatile;
		Int16 Or(Int16 num) volatile;
		Int16 And(Int16 num) volatile;

		operator Int16() { return mCount; }
	private:
		volatile Int16 mCount;
	};

	class SG_COMMON_API Atomic32
	{
	public:
		Atomic32()
			:mCount(0)
		{}
		~Atomic32() = default;

		long Increase() volatile;
		long Decrease() volatile;
		long Add(long num) volatile;
		long Exchange(long num) volatile;
		long Or(long num) volatile;
		long And(long num) volatile;
		
		operator int() { return mCount; }
	private:
		volatile int mCount;
 	};

	class SG_COMMON_API Atomic64
	{
	public:
		Atomic64()
			:mCount(0)
		{}
		~Atomic64() = default;

		Int64 Increase() volatile;
		Int64 Decrease() volatile;
		Int64 Add(Int64 num) volatile;
		Int64 Exchange(Int64 num) volatile;
		Int64 Or(Int64 num) volatile;
		Int64 And(Int64 num) volatile;

		operator Int64() { return mCount; }
	private:
		volatile Int64 mCount;
	};

	class SG_COMMON_API Semaphore
	{
	public:
		Semaphore(int maximunCnt, int initCnt = 0);
		~Semaphore();

		void Acquire();
		void Release();
	private:
		struct SG_COMMON_API InternalSemaphore
		{
			InternalSemaphore(int maximunCnt, int initCnt = 0);
			~InternalSemaphore();

			void Acquire();
			void Release();

			void* mHandle;
		};
	private:
		InternalSemaphore mSemaphore;
		Atomic32 mCount;
	};

}