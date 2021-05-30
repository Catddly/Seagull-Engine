#pragma once

#include "Common/Core/Defs.h"
#include "Common/Memory/IMemory.h"
#include "Core/STL/type_traits.h"

namespace SG
{

	template <typename T, typename... Args>
	T* PlacementNew(void* ptr, Args&&... args)
	{
		return new (ptr)T(SG::forward<Args>(args)...);
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

namespace impl
{
	template<class ForwardIterator>
	SG_INLINE void destruct_impl(ForwardIterator beg, ForwardIterator end, true_type)
	{
		// has trivial destructor, do nothing
	}

	template<class ForwardIterator>
	SG_INLINE void destruct_impl(ForwardIterator beg, ForwardIterator end, false_type)
	{
		typedef typename iterator_traits<ForwardIterator>::value_type value_type;
		while (beg != end)
		{
			(*beg).~value_type();
			++beg;	
		}
	}
}

	//! Call the destructor of [beg, end) if it has a non-trivial destructor
	template<class ForwardIterator>
	SG_INLINE void Destruct(ForwardIterator beg, ForwardIterator end)
	{
		typedef typename iterator_traits<ForwardIterator>::value_type value_type;
		impl::destruct_impl(beg, end, has_trivial_destructor<value_type>());
	}

}