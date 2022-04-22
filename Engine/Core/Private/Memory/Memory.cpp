#include "StdAfx.h"
#include "Memory/Memory.h"

#include "Profile/Profile.h"

// TODO: add memory tracking
namespace SG
{

	void* Memory::Malloc(Size size) noexcept
	{
		void* pNew = mi_malloc(size);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, size, "unknown", __LINE__, "unknown");
		SG_PROFILE_ALLOC(pNew, size);
#endif
		return pNew;
	}

	void* Memory::Malloc(Size size, const char* file, UInt32 line, const char* function) noexcept
	{
		void* pNew = mi_malloc(size);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, size, file, line, function);
		SG_PROFILE_ALLOC(pNew, size);
#endif
		return pNew;
	}

	void* Memory::MallocAlign(Size size, Size alignment) noexcept
	{
		void* pNew = mi_malloc_aligned(size, alignment);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, size, "unknown", __LINE__, "unknown");
		SG_PROFILE_ALLOC(pNew, size);
#endif
		return pNew;
	}

	void* Memory::Calloc(Size count, Size size) noexcept
	{
		void* pNew = mi_calloc(count, size);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, count * size, "unknown", __LINE__, "unknown");
		SG_PROFILE_ALLOC(pNew, size * count);
#endif
		return pNew;
	}

	void* Memory::CallocAlign(Size count, Size size, Size alignment) noexcept
	{
		void* pNew = mi_calloc_aligned(count, size, alignment);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, count * size, "unknown", __LINE__, "unknown");
		SG_PROFILE_ALLOC(pNew, count * size);
#endif
		return pNew;
	}

	void* Memory::Realloc(void* ptr, Size newSize) noexcept
	{
		void* pNew = mi_realloc(ptr, newSize);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, newSize, "unknown", __LINE__, "unknown");
		SG_PROFILE_ALLOC(pNew, newSize);
#endif
		return pNew;
	}

	void* Memory::ReallocAlign(void* ptr, Size newSize, Size alignment) noexcept
	{
		void* pNew = mi_realloc_aligned(ptr, newSize, alignment);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, newSize, "unknown", __LINE__, "unknown");
		SG_PROFILE_ALLOC(pNew, newSize);
#endif
		return pNew;
	}

	void  Memory::Free(void* ptr) noexcept
	{
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::ReleaseMemory(ptr);
		SG_PROFILE_FREE(ptr);
#endif
		mi_free(ptr);
	}

	void  Memory::FreeAlign(void* ptr, Size alignment) noexcept
	{
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::ReleaseMemory(ptr);
		SG_PROFILE_FREE(ptr);
#endif
		mi_free_aligned(ptr, alignment);
	}

}