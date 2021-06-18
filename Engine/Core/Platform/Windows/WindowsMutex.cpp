#include "StdAfx.h"
#include "WindowsMutex.h"

namespace SG
{

	CWindowsMutex::CWindowsMutex()
	{
		OnInit();
	}

	CWindowsMutex::~CWindowsMutex()
	{
		Release();
	}

	bool CWindowsMutex::OnInit()
	{
		return ::InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)&mHandle, 1500);
	}

	void CWindowsMutex::OnDestroy()
	{
		::DeleteCriticalSection((CRITICAL_SECTION*)&mHandle);
	}

	void CWindowsMutex::Acquire()
	{
		::EnterCriticalSection((CRITICAL_SECTION*)&mHandle);
	}

	bool CWindowsMutex::TryAcquire()
	{
		return ::TryEnterCriticalSection((CRITICAL_SECTION*)&mHandle);
	}

	void CWindowsMutex::Release()
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)&mHandle);
	}

	CMutexLock::CMutexLock(CWindowsMutex& mutex)
		:mMutex(mutex)
	{
		mMutex.Acquire();
	}

	CMutexLock::~CMutexLock()
	{
		mMutex.Release();
	}

}