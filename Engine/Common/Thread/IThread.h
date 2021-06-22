#pragma once

#include "Common/Config.h"
#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

#include "Common/Stl/string.h"

namespace SG
{

#ifdef __cplusplus
extern "C"
{
#endif

	typedef void*  ThreadHandle;
	typedef UInt32 ThreadID;
	typedef void (*ThreadFunc)(void* pUser);

	typedef struct Thread
	{
		ThreadHandle  pHandle;
		ThreadFunc    pFunc;
		void*         pUser;
	} Thread;

	SG_COMMON_API bool        ThreadCreate(Thread* pThread, ThreadFunc func, void* pUser);
	SG_COMMON_API void        ThreadRestore(Thread* pThread);

	SG_COMMON_API void        ThreadSuspend(Thread* pThread);
	SG_COMMON_API void        ThreadJoin(Thread* pThread);

	SG_COMMON_API UInt32      GetNumCPUCores();
	SG_COMMON_API void        ThreadSleep(UInt32 ms); 	//!< Thread sleep in milliseconds.

	SG_COMMON_API UInt32      GetThreadID(Thread* pThread);
	SG_COMMON_API UInt32      GetCurrThreadID();

	SG_COMMON_API const char* GetCurrThreadName();
	SG_COMMON_API void        SetCurrThreadName(const char* name);

#ifdef __cplusplus
}
#endif

	class Mutex;
	class SG_COMMON_API ScopeLock
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
#	include "Common/Thread/IThread_Windows.h"
#endif
