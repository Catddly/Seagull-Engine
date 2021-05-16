#pragma once

#include "Common/Core/Defs.h"
#include "Core/STL/type_traits.h"

#include <mimalloc/include/mimalloc.h>

namespace SG
{

	template <typename T, typename... Args>
	T* PlacementNew(void* ptr, Args&&... args)
	{
		return new (ptr)T(SG::forward<Args>(args)...);
	}

	template <typename T, typename... Args> T* New(Args&&... args)
	{
		T* ptr = reinterpret_cast<T*>(mi_malloc(sizeof(T)));
		return PlacementNew<T>(ptr, SG::forward<Args>(args)...);
	}

	template <typename T, typename... Args> T* NewAlign(Size alignment, Args&&... args)
	{
		T* ptr = reinterpret_cast<T*>(mi_malloc_aligned(sizeof(T), alignment));
		return PlacementNew<T>(ptr, SG::forward<Args>(args)...);
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

	template <typename T> T* MallocT() { Malloc(sizeof(T)); };
	template <typename T> T* CallocT(Size count) { Calloc(count, sizeof(T)); };

namespace impl
{
	template<class ForwardIterator>
	SG_INLINE void destruct_impl(ForwardIterator beg, ForwardIterator end, true_type())
	{
		// has trivial destructor, do nothing
	}

	template<class ForwardIterator>
	SG_INLINE void destruct_impl(ForwardIterator beg, ForwardIterator end, false_type())
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
		destruct_impl(beg, end, has_trivial_destructor<value_type>());
	}

}