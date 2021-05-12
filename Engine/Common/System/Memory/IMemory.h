#pragma once

#include "Common/Base/BasicTypes.h"

namespace SG
{

	void* Malloc(Size size) noexcept;
	void* MallocAlign(Size size, Size alignment) noexcept;
	void* Calloc(Size count, Size size) noexcept;
	void* CallocAlign(Size count, Size size, Size alignment) noexcept;
	void* Realloc(void* ptr, Size newSize) noexcept;
	void* ReallocAlign(void* ptr, Size newSize, Size alignment) noexcept;

	void  Free(void* ptr) noexcept;

	template <typename T, typename... Args> T* New(Args&&... args);
	template <typename T, typename... Args> T* NewAlign(Size alignment, Args&&... args);
	void* NewNothrow(Size size) noexcept;
	void* NewAlignNothrow(Size size, Size alignment) noexcept;

	template <typename T> void Delete(T* ptr);
	template <typename T> void DeleteNothrow(T* ptr) noexcept;

	template <typename T> T* MallocT()           { Malloc(sizeof(T)); };
	template <typename T> T* CallocT(Size count) { Calloc(count, sizeof(T)); };
}