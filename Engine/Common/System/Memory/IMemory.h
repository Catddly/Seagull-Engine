#pragma once
#include "Common/Config.h"

#include "Common/Base/BasicTypes.h"

namespace SG
{

	SG_COMMON_API void* Malloc(Size size) noexcept;
	SG_COMMON_API void* MallocAlign(Size size, Size alignment) noexcept;
	SG_COMMON_API void* Calloc(Size count, Size size) noexcept;
	SG_COMMON_API void* CallocAlign(Size count, Size size, Size alignment) noexcept;
	SG_COMMON_API void* Realloc(void* ptr, Size newSize) noexcept;
	SG_COMMON_API void* ReallocAlign(void* ptr, Size newSize, Size alignment) noexcept;

	SG_COMMON_API void  Free(void* ptr) noexcept;

	template <typename T, typename... Args> SG_COMMON_API T* New(Args&&... args);
	template <typename T, typename... Args> SG_COMMON_API T* NewAlign(Size alignment, Args&&... args);
	SG_COMMON_API void* NewNothrow(Size size) noexcept;
	SG_COMMON_API void* NewAlignNothrow(Size size, Size alignment) noexcept;

	template <typename T> SG_COMMON_API void Delete(T* ptr);
	template <typename T> SG_COMMON_API void DeleteNothrow(T* ptr) noexcept;

	template <typename T> SG_COMMON_API T* MallocT();
	template <typename T> SG_COMMON_API T* CallocT(Size count);

	template <typename T> T* MallocT() { Malloc(sizeof(T)); };
	template <typename T> T* CallocT(Size count) { Calloc(count, sizeof(T)); };
}