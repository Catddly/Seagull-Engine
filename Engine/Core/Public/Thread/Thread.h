#pragma once

#include "Core/Config.h"
#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

#include "Stl/string.h"

namespace SG
{

	typedef void*  ThreadHandle;
	typedef UInt32 ThreadID;
	typedef void (*ThreadFunc)(void* pUser);

	typedef struct Thread
	{
		ThreadHandle  pHandle;
		ThreadFunc    pFunc;
		void*         pUser;
	} Thread;

	SG_CORE_API bool        ThreadCreate(Thread* pThread, ThreadFunc func, void* pUser);
	SG_CORE_API void        ThreadRestore(Thread* pThread);

	SG_CORE_API void        ThreadSuspend(Thread* pThread);
	SG_CORE_API void        ThreadJoin(Thread* pThread);

	SG_CORE_API UInt32      GetNumCPUCores();
	SG_CORE_API void        ThreadSleep(UInt32 ms); 	//!< Thread sleep in milliseconds.

	SG_CORE_API UInt32      GetThreadID(Thread* pThread);
	SG_CORE_API UInt32      GetCurrThreadID();

	SG_CORE_API const char* GetCurrThreadName();
	SG_CORE_API void        SetCurrThreadName(const char* name);

	class Mutex;
	class SG_CORE_API ScopeLock
	{
	public:
		ScopeLock(Mutex& mutex);
		~ScopeLock();
		SG_CLASS_NO_COPY_ASSIGNABLE(ScopeLock);

	private:
		Mutex& mMutex;
	};
}

#ifdef SG_PLATFORM_WINDOWS
#	include "Thread/Thread.Windows.h"
#endif
