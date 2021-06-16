#pragma once

#include "Common/Platform/IThread.h"

namespace SG
{

	struct SWindowsThreadOp : public IThreadOp
	{
		virtual bool    CreateThread(ThreadFunc func, void* pUser, SThread* pThread) override;
		virtual void    RestoreThread(SThread* pThread) override;

		virtual void    SuspendThread(SThread* pThread) override;
		virtual void    JoinThread(SThread* pThread) override;

		virtual UInt32  GetNumCPUCores() const override;
		//! Thread sleep in miliseconds.
		virtual void    Sleep(UInt32 ms) const override;
		virtual bool    GetThreadID(SThread* pThread) override;
	};

}