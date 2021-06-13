#include "StdAfx.h"
#include "Common/Memory/IMemory.h"

#include <mimalloc/include/mimalloc.h>

// TODO: add memory tracking
void* operator new(size_t n)
{
	//printf("Allocate %llu\n", n);
	return mi_new(n); 
}

void* operator new(size_t n, const nothrow_t& tag) 
{
	//printf("Allocate %d\n", n);
	return mi_new_nothrow(n); 
}

void* operator new(size_t n, align_val_t align)
{
	//printf("Allocate %d\n", n);
	return mi_new_aligned(n, (size_t)align);
}

void* operator new[](size_t n) 
{
	//printf("Allocate %d\n", n);
	return mi_new(n); 
}

void* operator new[](size_t n, const nothrow_t& tag) 
{
	//printf("Allocate %d\n", n);
	return mi_new_nothrow(n); 
}

void* operator new[](size_t n, align_val_t align)
{
	//printf("Allocate %d\n", n);
	return mi_new_aligned(n, (size_t)align);
}

void operator delete(void* p)
{
	mi_free(p);
}

void operator delete(void* p, size_t n) noexcept
{ 
	mi_free(p);
}

void operator delete(void* p, const nothrow_t& tag) noexcept
{
	mi_free(p);
}

void operator delete(void* ptr, align_val_t align) noexcept
{
	mi_free_aligned(ptr, (size_t)align);
}

void operator delete[](void* p)
{
	mi_free(p);
}

void operator delete[](void* p, size_t n) noexcept
{ 
	mi_free(p); 
}

void operator delete[](void* p, const nothrow_t& tag) noexcept
{
	mi_free(p);
}

void operator delete[](void* ptr, align_val_t align) noexcept
{
	mi_free_aligned(ptr, (size_t)align);
}

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