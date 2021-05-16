#include "StdAfx.h"
#include "Common/System/Memory/IMemory.h"

#include <mimalloc/include/mimalloc.h>

namespace SG
{

	void* Malloc(Size size) noexcept
	{
		return mi_malloc(size);
	}

	void* MallocAlign(Size size, Size alignment) noexcept
	{
		return mi_malloc_aligned(size, alignment);
	}

	void* Calloc(Size count, Size size) noexcept
	{
		return mi_calloc(count, size);
	}

	void* CallocAlign(Size count, Size size, Size alignment) noexcept
	{
		return mi_calloc_aligned(count, size, alignment);
	}

	void* Realloc(void* ptr, Size newSize) noexcept
	{
		return mi_realloc(ptr, newSize);
	}

	void* ReallocAlign(void* ptr, Size newSize, Size alignment) noexcept
	{
		return mi_realloc_aligned(ptr, newSize, alignment);
	}

	void Free(void* ptr) noexcept
	{
		mi_free(ptr);
	}

}