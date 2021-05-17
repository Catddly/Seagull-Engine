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
}