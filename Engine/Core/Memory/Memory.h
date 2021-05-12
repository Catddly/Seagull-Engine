#pragma once

#include "Common/Core/Defs.h"
#include "Common/System/Memory/IMemory.h"

#include <mimalloc/include/mimalloc.h>

namespace SG
{

	template <typename T, typename... Args>
	static T* PlacementNew(void* ptr, Args&&... args)
	{
		return new (ptr)T(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args> T* New(Args&&... args)
	{
		T* ptr = reinterpret_cast<T*>(mi_malloc(sizeof(T)));
		return PlacementNew<T>(ptr, std::forward<Args>(args)...);
	}

	template <typename T, typename... Args> T* NewAlign(Size alignment, Args&&... args)
	{
		T* ptr = reinterpret_cast<T*>(mi_malloc_aligned(sizeof(T), alignment));
		return PlacementNew<T>(ptr, std::forward<Args>(args)...);
	}

	template <typename T> void Delete(T* ptr)
	{
		if (ptr)
		{
			ptr->~T();
			mi_free(ptr);
		}
	}

	template <typename T> void DeleteNothrow(T* ptr) noexcept
	{
		if (ptr)
		{
			ptr->~T();
			mi_free(ptr);
		}
	}

}