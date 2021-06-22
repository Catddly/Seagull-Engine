#pragma once

#include "Common/Core/Defs.h"
#include "Common/Thread/IThread.h"

namespace SG
{

	//! A handle to a job, be used to establish dependency,
	//! do synchronization and job data deletion.
	struct SJobHandle
	{
		void Complete();
		void Dispose();
	};

	struct SJobStatus
	{

	};

	template<class InData, class OutData>
	struct IJob
	{
		virtual void Execute(const InData& inData, OutData& outData) = 0;
	};

}