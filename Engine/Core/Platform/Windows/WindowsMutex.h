#pragma once

#include "Core/Config.h"
#include "Common/Thread/IThread.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace SG
{

	class SG_CORE_API CWindowsMutex : public IMutex
	{
	public:
		CWindowsMutex();
		~CWindowsMutex();

		virtual bool OnInit() override;
		virtual void OnDestroy() override;

		virtual void Acquire() override;
		virtual bool TryAcquire() override;
		virtual void Release() override;
	private:
		CRITICAL_SECTION mHandle;
	};

	class SG_CORE_API CMutexLock
	{
	public:
		CMutexLock(CWindowsMutex& mutex);
		~CMutexLock();
		
		CMutexLock(const CMutexLock&) = delete;
		CMutexLock operator=(const CMutexLock&) = delete;
	private:
		CWindowsMutex& mMutex;
	};

}