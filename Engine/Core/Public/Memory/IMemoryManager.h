#pragma once

#include "Base/BasicTypes.h"

namespace SG
{

	struct IMemoryManager
	{
		typedef UInt8 HeapHandle;
		enum { SG_BAD_HEAP_HANDLE = 0xff };

		struct SMemoryInfo
		{
			//! the memory that the user had
			UInt64 totalAllocated;
			//! the memory that the user had retured
			UInt64 totalFredMemory;
			//! the memory that the core owned
			UInt64 totalMemory;
		};

		virtual ~IMemoryManager() = default;
	};

}