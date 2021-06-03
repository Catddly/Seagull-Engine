#pragma once

#include "Common/Core/Defs.h"

#include "Common/Memory/IMemory.h"

#include <vcruntime_new.h> // for correct placement new operation

namespace SG
{

	template <typename T, typename... Args>
	T* PlacementNew(void* ptr, Args&&... args)
	{
		return ::new (ptr)T(SG::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	T* New(Args&&... args)
	{
		T* ptr = reinterpret_cast<T*>(Malloc(sizeof(T)));
		return PlacementNew<T>(ptr, SG::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	T* NewAlign(Size alignment, Args&&... args)
	{
		T* ptr = reinterpret_cast<T*>(MallocAlign(sizeof(T), alignment));
		return PlacementNew<T>(ptr, SG::forward<Args>(args)...);
	}

	//! Safe memory delete for ptr
	template <typename T> void Delete(T* ptr)
	{
		if (ptr)
		{
			ptr->~T();
			Free(ptr);
		}
	}

	//! Safe memory delete for ptr (noexcept)
	template <typename T> void DeleteNothrow(T* ptr) noexcept
	{
		if (ptr)
		{
			ptr->~T();
			Free(ptr);
		}
	}

	template <typename T> T* MallocT() { Malloc(sizeof(T)); };
	template <typename T> T* CallocT(Size count) { Calloc(count, sizeof(T)); };

}