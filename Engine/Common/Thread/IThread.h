#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

#include "Common/Stl/string.h"

namespace SG
{

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct IMutex
	{
		virtual ~IMutex() = default;

		virtual bool OnInit() = 0;
		virtual void OnDestroy() = 0;

		virtual void Acquire() = 0;
		virtual bool TryAcquire() = 0;
		virtual void Release() = 0;
	} IMutex;

	typedef void*  ThreadHandle;
	typedef UInt32 ThreadID;
	typedef void (*ThreadFunc)(void* pUser);

	typedef struct SThread
	{
		ThreadHandle  pHandle;
		ThreadFunc    pFunc;
		void*         pUser;
		ThreadID      id;
	} SThread;

	SG_COMMON_API bool    CreThread(SThread* pThread, ThreadFunc func, void* pUser);
	SG_COMMON_API void    RestoreThread(SThread* pThread);

	SG_COMMON_API void    SusThread(SThread* pThread);
	SG_COMMON_API void    JoinThread(SThread* pThread);

	SG_COMMON_API UInt32  GetNumCPUCores();
		//! Thread sleep in miliseconds.
	SG_COMMON_API void    ThreadSleep(UInt32 ms);

	SG_COMMON_API bool    GetThreadID(SThread* pThread);
	SG_COMMON_API UInt32  GetCurrThreadID();

	SG_COMMON_API const char* GetCurrThreadName();
	SG_COMMON_API void SetCurrThreadName(const char* name);

#ifdef __cplusplus
}
#endif

}