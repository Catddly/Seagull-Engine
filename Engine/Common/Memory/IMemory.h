#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

struct nothrow_t
{
	explicit nothrow_t() = default;
};
enum class align_val_t : size_t {};

// overload global new and delete
void* operator new(size_t n);
void* operator new(size_t n, const nothrow_t& tag); // C++ 11
void* operator new(size_t n, align_val_t align); // C++ 17
void* operator new[](size_t n);
void* operator new[](size_t n, const nothrow_t& tag); // C++ 11
void* operator new[](size_t n, align_val_t align); // C++ 17

void  operator delete(void* p) noexcept;
void  operator delete(void* p, size_t n) noexcept;
void  operator delete(void* p, const nothrow_t& tag) noexcept; // only be called when new_nothrow is been called
void  operator delete(void* ptr, align_val_t align) noexcept;
void  operator delete[](void* p) noexcept;
void  operator delete[](void* p, size_t n) noexcept;
void  operator delete[](void* p, const nothrow_t& tag) noexcept;
void  operator delete[](void* ptr, align_val_t align) noexcept;

namespace SG
{
	SG_COMMON_API void* Malloc(Size size) noexcept;
	SG_COMMON_API void* MallocAlign(Size size, Size alignment) noexcept;
	SG_COMMON_API void* Calloc(Size count, Size size) noexcept;
	SG_COMMON_API void* CallocAlign(Size count, Size size, Size alignment) noexcept;
	SG_COMMON_API void* Realloc(void* ptr, Size newSize) noexcept;
	SG_COMMON_API void* ReallocAlign(void* ptr, Size newSize, Size alignment) noexcept;

	SG_COMMON_API void  Free(void* ptr) noexcept;
}