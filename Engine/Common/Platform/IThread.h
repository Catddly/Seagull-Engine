#pragma once

#include "Common/Base/BasicTypes.h"
//#include "../Base/BasicTypes.h"

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

	typedef struct SThread
	{
		ThreadHandle  pHandle;
		ThreadFunc    pFunc;
		void*         pUser;
		ThreadID      id;
		eastl::string name = "Null";
	} SThread;

	typedef struct IThreadOp
	{
		virtual bool    CreateThread(ThreadFunc func, void* pUser, SThread* pThread) = 0;
		virtual void    RestoreThread(SThread* pThread) = 0;

		virtual void    SuspendThread(SThread* pThread) = 0;
		virtual void    JoinThread(SThread* pThread) = 0;

		virtual UInt32  GetNumCPUCores() const = 0;
		//! Thread sleep in miliseconds.
		virtual void    Sleep(UInt32 ms) const = 0;

		virtual bool    GetThreadID(SThread* pThread) = 0;
	} IThreadOp;

#ifdef __cplusplus
}
#endif

}