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

	template <bool isMove, typename InputIterator, typename OutputIterator>
	SG_INLINE OutputIterator move_and_copy_chooser(InputIterator first, InputIterator last, OutputIterator result)
	{
		typedef typename iterator_traits<InputIterator>::iterator_category  IIC;
		typedef typename iterator_traits<OutputIterator>::iterator_category OIC;
		typedef typename iterator_traits<InputIterator>::value_type         value_type_input;
		typedef typename iterator_traits<OutputIterator>::value_type        value_type_output;

		const bool canBeMemmoved = is_trivially_copyable<value_type_output>::value &&
			is_same<value_type_input, value_type_output>::value &&
			(is_pointer<InputIterator>::value || is_same<IIC, contiguous_iterator_tag>::value) &&
			(is_pointer<OutputIterator>::value || is_same<OIC, contiguous_iterator_tag>::value);

		return nullptr;
		//return move_and_copy_helper<IIC, isMove, canBeMemmoved>::move_or_copy(first, last, result); // Need to chose based on the input iterator tag and not the output iterator tag, because containers accept input ranges of iterator types different than self.
	}

	// We have a second layer of unwrap_iterator calls because the original iterator might be something like move_iterator<generic_iterator<int*> > (i.e. doubly-wrapped).
	template <bool isMove, typename InputIterator, typename OutputIterator>
	SG_INLINE OutputIterator move_and_copy_unwrapper(InputIterator first, InputIterator last, OutputIterator result)
	{
		return OutputIterator(move_and_copy_chooser<isMove>(unwrap_iterator(first), unwrap_iterator(last), unwrap_iterator(result))); // Have to convert to OutputIterator because result.base() could be a T*
	}

	template <typename InputIterator, typename OutputIterator>
	SG_INLINE OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
	{
		const bool isMove = is_move_iterator_v<InputIterator>; 
		SG_NO_USE(isMove);

		return move_and_copy_unwrapper<isMove>(unwrap_iterator(first), unwrap_iterator(last), result);
	}

	template <typename InputIterator, typename ForwardIterator>
	SG_INLINE ForwardIterator copy_ptr_impl(InputIterator first, InputIterator last, ForwardIterator dst, true_type)
	{
		return copy(first, last, dst);
	}

} // namespace impl

	//! Copy [beg, end) to [res + (beg - end)). 
	//! Check if it is movable or copyable
	template<class Begin, class End, class Result>
	SG_INLINE Result* copy_ptr(Begin beg, End end, Result res)
	{
		typedef typename iterator_traits<generic_iterator<Result, void>>::value_type value_type;
		const generic_iterator<Result, void> iter(impl::copy_ptr_impl(generic_iterator<Begin, void>(beg),
			generic_iterator<End, void>(end),
			generic_iterator<Result, void>(res),
			is_trivially_copy_assignable<value_type>()));
		// return the raw pointer of generic_iterator
		return iter.base();
	}

} // namespace SG

#endif // COPY_AND_MOVE_H
