#pragma once

#include "Common/Config.h"

namespace SG
{

	struct CriticalSection
	{
		void* debugInfo;
		long  lockCount;
		long  recursionCount;
		void* owningThread;
		void* lockSemaphore;
		unsigned long* spinCount;
	};

	class SG_COMMON_API Mutex
	{
	public:
		Mutex();
		~Mutex();

		void Lock();
		bool TryLock();
		void UnLock();
	private:
		CriticalSection mHandle;
	};

}