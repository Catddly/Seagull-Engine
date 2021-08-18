#include "StdAfx.h"
#include "Common/Memory/IMemory.h"

#include <mimalloc/include/mimalloc.h>

//#include <stdio.h>

// TODO: add memory tracking
//void* operator new(size_t n)
//{
//	//printf("Allocate %llu\n", n);
//	return mi_new(n); 
//}
//
//void* operator new(size_t n, const nothrow_t& tag) 
//{
//	//printf("Allocate %llu\n", n);
//	return mi_new_nothrow(n); 
//}
//
//void* operator new(size_t n, align_val_t align)
//{
//	//printf("Allocate %llu\n", n);
//	return mi_new_aligned(n, (size_t)align);
//}
//
//void* operator new[](size_t n) 
//{
//	//printf("Allocate %llu\n", n);
//	return mi_new(n); 
//}
//
//void* operator new[](size_t n, const nothrow_t& tag) 
//{
//	//printf("Allocate %llu\n", n);
//	return mi_new_nothrow(n); 
//}
//
//void* operator new[](size_t n, align_val_t align)
//{
//	//printf("Allocate %llu\n", n);
//	return mi_new_aligned(n, (size_t)align);
//}
//
//void operator delete(void* p)
//{
//	mi_free(p);
//}
//
//void operator delete(void* p, size_t n) noexcept
//{ 
//	mi_free(p);
//}
//
//void operator delete(void* p, const nothrow_t& tag) noexcept
//{
//	mi_free(p);
//}
//
//void operator delete(void* ptr, align_val_t align) noexcept
//{
//	mi_free_aligned(ptr, (size_t)align);
//}
//
//void operator delete[](void* p)
//{
//	mi_free(p);
//}
//
//void operator delete[](void* p, size_t n) noexcept
//{ 
//	mi_free(p); 
//}
//
//void operator delete[](void* p, const nothrow_t& tag) noexcept
//{
//	mi_free(p);
//}
//
//void operator delete[](void* ptr, align_val_t align) noexcept
//{
//	mi_free_aligned(ptr, (size_t)align);
//}

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