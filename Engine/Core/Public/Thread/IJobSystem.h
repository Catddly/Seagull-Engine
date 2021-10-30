#pragma once

#include "Defs/Defs.h"
#include "Thread/Thread.h"

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

	/// maybe we want a decorator to be a simple wrapper to IJob
	/// to add some synchronization function to its handle.

	//! Data-oriented structure, per data executable function.
	template<class InData, class OutData>
	interface IJob
	{
		virtual void Execute(const InData& inData, OutData& outData) = 0;
	};

}