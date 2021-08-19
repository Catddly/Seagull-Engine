#include "StdAfx.h"
#include "Common/Memory/IMemory.h"

#include <mimalloc/include/mimalloc.h>

// TODO: add memory tracking
namespace SG
{

	void* Memory::Malloc(Size size) noexcept
	{
		return mi_malloc(size);
	}

	void* Memory::MallocAlign(Size size, Size alignment) noexcept
	{
		return mi_malloc_aligned(size, alignment);
	}

	void* Memory::Calloc(Size count, Size size) noexcept
	{
		return mi_calloc(count, size);
	}

	void* Memory::CallocAlign(Size count, Size size, Size alignment) noexcept
	{
		return mi_calloc_aligned(count, size, alignment);
	}

	void* Memory::Realloc(void* ptr, Size newSize) noexcept
	{
		return mi_realloc(ptr, newSize);
	}

	void* Memory::ReallocAlign(void* ptr, Size newSize, Size alignment) noexcept
	{
		return mi_realloc_aligned(ptr, newSize, alignment);
	}

	void  Memory::Free(void* ptr) noexcept
	{
		mi_free(ptr);
	}

	void  Memory::FreeAlign(void* ptr, Size alignment) noexcept
	{
		mi_free_aligned(ptr, alignment);
	}

}