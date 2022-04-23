#include "StdAfx.h"
#include "Memory/Memory.h"

#include "Profile/Profile.h"
#include "Math/MathBasic.h"

// TODO: add memory tracking
namespace SG
{

	void* Memory::Impl::MallocInternal(Size size) noexcept
	{
		void* pNew = mi_malloc(size);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, size, "unknown", __LINE__, "unknown");
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_ALLOC(pNew, size);
#endif
		return pNew;
	}

#if SG_ENABLE_MEMORY_TRACKING
	void* Memory::Malloc(Size size, const char* file, UInt32 line, const char* function) noexcept
	{
		void* pNew = mi_malloc(size);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, size, file, line, function);
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_ALLOC(pNew, size);
#endif
		return pNew;
	}
#endif

	void* Memory::Impl::MallocAlignInternal(Size size, Size alignment) noexcept
	{
		void* pNew = mi_malloc_aligned(size, alignment);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, size, "unknown", __LINE__, "unknown");
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_ALLOC(pNew, MinValueAlignTo(size, alignment));
#endif
		return pNew;
	}

	void* Memory::Impl::CallocInternal(Size count, Size size) noexcept
	{
		void* pNew = mi_calloc(count, size);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, count * size, "unknown", __LINE__, "unknown");
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_ALLOC(pNew, size * count);
#endif
		return pNew;
	}

	void* Memory::Impl::CallocAlignInternal(Size count, Size size, Size alignment) noexcept
	{
		void* pNew = mi_calloc_aligned(count, size, alignment);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, count * size, "unknown", __LINE__, "unknown");
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_ALLOC(pNew, MinValueAlignTo(size * count, alignment));
#endif
		return pNew;
	}

	void* Memory::Impl::ReallocInternal(void* ptr, Size newSize) noexcept
	{
		void* pNew = mi_realloc(ptr, newSize);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, newSize, "unknown", __LINE__, "unknown");
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_FREE(ptr);
		SG_PROFILE_ALLOC(pNew, newSize);
#endif
		return pNew;
	}

	void* Memory::Impl::ReallocAlignInternal(void* ptr, Size newSize, Size alignment) noexcept
	{
		void* pNew = mi_realloc_aligned(ptr, newSize, alignment);
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::TrackMemory(pNew, newSize, "unknown", __LINE__, "unknown");
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_FREE(ptr);
		SG_PROFILE_ALLOC(pNew, MinValueAlignTo(newSize, alignment));
#endif
		return pNew;
	}

	void  Memory::Impl::FreeInternal(void* ptr) noexcept
	{
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::ReleaseMemory(ptr);
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_FREE(ptr);
#endif
		mi_free(ptr);
	}

	void  Memory::Impl::FreeAlignInternal(void* ptr, Size alignment) noexcept
	{
#if SG_ENABLE_MEMORY_TRACKING
		MemoryRecorder::ReleaseMemory(ptr);
#endif
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_FREE(ptr);
#endif
		mi_free_aligned(ptr, alignment);
	}

}