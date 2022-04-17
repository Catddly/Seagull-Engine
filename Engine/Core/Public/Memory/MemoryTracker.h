#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "eastl/utility.h"

namespace SG
{

	struct MemoryTrackBlock
	{
		void*             memory = nullptr;
		Size              size = 0;
		MemoryTrackBlock* prev = nullptr;

		const char*  file;
		UInt32       line;
		const char*  function;
	};

	//typedef UInt8 HeapHandle;
	//enum { SG_BAD_HEAP_HANDLE = 0xff };

	//struct MemoryInfo
	//{
	//	//! The memory that had allocated
	//	UInt64 totalAllocated;
	//	//! The memory that the user had returned
	//	UInt64 totalFreedMemory;
	//};

	class SG_CORE_API MemoryScopeTracker
	{
	public:
		explicit MemoryScopeTracker(const char* scopeName);
		~MemoryScopeTracker();
	private:
#ifdef SG_DEBUG
		const char* mName;
#endif
	};

	class MemoryRecorder
	{
	public:
		SG_CORE_API static void TrackMemory(void* memory, Size size, const char* file, UInt32 line, const char* function);
		SG_CORE_API static void ReleaseMemory(void* memory);
	private:

		enum { MAX_SCOPE = 10 };
		friend class MemoryScopeTracker;

		static void Insert(MemoryTrackBlock* pBlock, MemoryTrackBlock* pEnd);
		static void Remove(void* pMemory);

		static void OnEnterScope(const char* scopeName);
		static void OnExitScope();
	private:
		static eastl::pair<const char*, MemoryTrackBlock*> mTrackStack[MAX_SCOPE];
		static Int32 mCurrStackPos;
		static MemoryTrackBlock* mpForgottenMemory;
	};

}