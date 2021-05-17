#ifndef COPY_AND_MOVE_H
#define COPY_AND_MOVE_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "../type_traits.h"
#include "iterator.h"

namespace SG
{
namespace impl
{
	//---------------------------------------------------------------------------------------------------------
	// 	   move_and_copy_helper
	//---------------------------------------------------------------------------------------------------------

	//! This helper will choose the right strategy for copy or move
	//! We can use memmove/memcpy if the following hold true:
	//! 1.InputIterator and OutputIterator are of the same type.
	//! 2.InputIterator and OutputIterator are of type contiguous_iterator_tag or simply are pointers (the two are virtually synonymous).
	//! 3.is_trivially_copyable<T>::value is true. (i.e. the constructor T(const T& t) (or T(T&& t) if present) can be replaced by memmove(this, &t, sizeof(T)))
	template<class IIC, bool isMove, bool canBeMemmoved>
	struct move_and_copy_helper
	{
		template <typename InputIterator, typename OutputIterator>
		static OutputIterator move_or_copy(InputIterator beg, InputIterator end, OutputIterator res) // default copy iterator
		{
			while (beg != end)
			{
				*res = *beg;
				++res; ++beg;
			}
			return res;
		}
	};

	// Specialization for copying non-trivial data via a random_access_iterator
	template<>
	struct move_and_copy_helper<random_access_iterator_tag, false, false>
	{
		template <typename InputIterator, typename OutputIterator>
		static OutputIterator move_or_copy(InputIterator beg, InputIterator end, OutputIterator res)
		{
			typedef typename iterator_traits<InputIterator>::difference_type difference_type;

			for (difference_type n = (end - beg); n > 0; --n, ++beg, ++res)
				*res = *beg;
			return res;
		}
	};

	// Specialization for moving non-trivial data via a random_access_iterator
	template<class IIC>
	struct move_and_copy_helper<IIC, false, false>
	{
		template <typename InputIterator, typename OutputIterator>
		static OutputIterator move_or_copy(InputIterator beg, InputIterator end, OutputIterator res)
		{
			while (beg != end)
			{
				*res = SG::move(*beg);
				++res; ++beg;
			}
			return res;
		}
	};

	// Specialization for moving non-trivial data via a random-access iterator
	// Here we detected move_iterator is present.
	// It's theoretically faster because the compiler can see the count when its a compile-time const.
	template <>
	struct move_and_copy_helper<random_access_iterator_tag, true, false>
	{
		template <typename InputIterator, typename OutputIterator>
		static OutputIterator move_or_copy(InputIterator beg, InputIterator end, OutputIterator res)
		{
			typedef typename iterator_traits<InputIterator>::difference_type difference_type;

			for (difference_type n = (end - beg); n > 0; --n, ++beg, ++res)
				*res = SG::move(*first);
			return res;
		}
	};

	// Specialization for that has the strict range overlap requirements
	template <bool isMove>
	struct move_and_copy_helper<random_access_iterator_tag, isMove, true>
	{
		template <typename T>
		static T* move_or_copy(const T* beg, const T* end, T* res)
		{
			// We could use memcpy here if there's no range overlap, but memcpy is rarely much faster than memmove.
			return (T*)::memmove(res, beg, (Size)((UIntPtr)end - (UIntPtr)beg)) + (end - beg);
		}
	};

	//---------------------------------------------------------------------------------------------------------
	// 	   move_and_copy_helper
	//---------------------------------------------------------------------------------------------------------

	template <bool isMove, typename InputIterator, typename OutputIterator>
	SG_INLINE OutputIterator move_and_copy_chooser(InputIterator beg, InputIterator end, OutputIterator res)
	{
		typedef typename iterator_traits<InputIterator>::iterator_category  IIC;
		typedef typename iterator_traits<OutputIterator>::iterator_category OIC;
		typedef typename iterator_traits<InputIterator>::value_type         value_type_input;
		typedef typename iterator_traits<OutputIterator>::value_type        value_type_output;

		const bool canBeMemmoved = is_trivially_copyable<value_type_output>::value &&
			is_same<value_type_input, value_type_output>::value &&
			(is_pointer<InputIterator>::value || is_same<IIC, contiguous_iterator_tag>::value) &&
			(is_pointer<OutputIterator>::value || is_same<OIC, contiguous_iterator_tag>::value);

		return move_and_copy_helper<IIC, isMove, canBeMemmoved>::move_or_copy(beg, end, res); // Need to chose based on the input iterator tag and not the output iterator tag, because containers accept input ranges of iterator types different than self.
	}

	// We have a second layer of unwrap_iterator calls because the original iterator might be something like move_iterator<generic_iterator<int*> > (i.e. doubly-wrapped).
	template <bool isMove, typename InputIterator, typename OutputIterator>
	SG_INLINE OutputIterator move_and_copy_unwrapper(InputIterator beg, InputIterator end, OutputIterator res)
	{
		// Have to convert to OutputIterator because result.base() could be a T*
		return OutputIterator(move_and_copy_chooser<isMove>(unwrap_iterator(beg), unwrap_iterator(end), unwrap_iterator(res))); 
	}

	template <typename InputIterator, typename OutputIterator>
	SG_INLINE OutputIterator move(InputIterator beg, InputIterator end, OutputIterator res)
	{
		return move_and_copy_unwrapper<true>(unwrap_iterator(beg), unwrap_iterator(end), res);
	}

	//! Simple unwrap function to unwrap the iterator and check if the InputIterator is a move_iterator
	template <typename InputIterator, typename OutputIterator>
	SG_INLINE OutputIterator copy(InputIterator beg, InputIterator end, OutputIterator res)
	{
		const bool isMove = is_move_iterator_v<InputIterator>; 
		SG_NO_USE(isMove);

		return move_and_copy_unwrapper<isMove>(unwrap_iterator(beg), unwrap_iterator(end), res);
	}

	//! If we should use trivially copy and assignable
	template <typename InputIterator, typename ForwardIterator>
	SG_INLINE ForwardIterator copy_ptr_impl(InputIterator beg, InputIterator end, ForwardIterator dst, true_type)
	{
		return copy(beg, end, dst);
	}

	//! If we should use trivially copy and assignable
	template <typename InputIterator, typename ForwardIterator>
	inline ForwardIterator copy_ptr_impl(InputIterator beg, InputIterator end, ForwardIterator dst, false_type)
	{
		typedef typename iterator_traits<ForwardIterator>::value_type value_type;
		ForwardIterator currentDst(dst);

		// because it is non-trivially copy assignable, maybe in the process of construction,
		// constructor may throw
#if SG_ENABLE_EXCEPTION
		try
		{
#endif
			while (beg != end)
			{
				// placement new to construct each object to dst
				::new(static_cast<void*>(SG::addressof(*currentDst))) value_type(*beg);
				++beg; ++currentDst;
			}
#if SG_ENABLE_EXCEPTION
		}
		catch (...)
		{
			for (; dst < currentDst; ++dst)
				(*dst).~value_type();
			throw;
		}
#endif
		return currentDst;
	}

} // namespace impl

	//! Copy [beg, end) to [res + (beg - end)). 
	//! Check if it is movable or copyable.
	//! This function is for uninitialized type or data to use. (i.e. uninitialized_move for iterators)
	//! If you don't want to use this, use copy() in "algorithm.h" 
	template<class Begin, class End, class Result>
	SG_INLINE Result copy_ptr_uninitialzied(Begin beg, End end, Result res)
	{
		typedef typename iterator_traits<generic_iterator<Result, void>>::value_type value_type;
		// here we don't know if the Begin, End or Result is iterator,
		// wrapped with the generic_iterator to make them act like a iterator.
		const generic_iterator<Result, void> iter(impl::copy_ptr_impl(generic_iterator<Begin, void>(beg),
			generic_iterator<End, void>(end),
			generic_iterator<Result, void>(res),
			is_trivially_copy_assignable<value_type>()));

		return iter.base(); // return the raw pointer of generic_iterator
	}

} // namespace SG

#endif // COPY_AND_MOVE_H
