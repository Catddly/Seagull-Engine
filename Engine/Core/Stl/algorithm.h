#ifndef ALGORITHM_H
#define ALGORITHM_H

#pragma once

#include "Common/Core/Defs.h"
#include "type_traits.h"

namespace SG
{

	//! If the content in begin1 to end1 is equal to begin2 (compile-time)
	template<class Iter1, class Iter2>
	SG_CONSTEXPR SG_INLINE bool 
	equal(Iter1 begin1, Iter1 end1, Iter2 begin2)
	{
		for (; begin1 != end1; ++begin1, ++begin2)
			if (!(*begin1 == *begin2))
				return false;
		return true;
	}

	//! If the content in begin1 to end1 is equal to begin2 (runtime)
	template<class Iter1, class Iter2, class ComparaFunc>
	SG_INLINE bool equal(Iter1 begin1, Iter1 end1, Iter2 begin2, ComparaFunc func)
	{
		for (; begin1 != end1; ++begin1, ++begin2)
			if (!func(*begin1, *begin2))
				return false;
		return true;
	}

	//! Find the bigger value between lhs and rhs. (compile-time)
	//! Use smax to avoid collusion to MSVC's max.
	template <typename T>
	SG_INLINE SG_CONSTEXPR typename enable_if<is_scalar<T>::value, T>::type
	smax(T lhs, T rhs)
	{
		return lhs < rhs ? rhs : lhs;
	}

	//! Find the bigger value between lhs and rhs. (runtime)
	//! Use smax to avoid collusion to MSVC's max.
	template <typename T>
	SG_INLINE typename enable_if<!is_scalar<T>::value, const T&>::type
	smax(const T& lhs, const T& rhs)
	{
		return lhs < rhs ? rhs : lhs;
	}

	//! Find the smaller value between lhs and rhs. (compile-time)
	//! Use smin to avoid collusion to MSVC's min.
	template <typename T>
	SG_INLINE SG_CONSTEXPR typename enable_if<is_scalar<T>::value, T>::type
	smin(T lhs, T rhs)
	{
		return lhs > rhs ? rhs : lhs;
	}

	//! Find the smaller value between lhs and rhs. (runtime)
	//! Use smin to avoid collusion to MSVC's min.
	template <typename T>
	SG_INLINE typename enable_if<!is_scalar<T>::value, const T&>::type
	smin(const T& a, const T& b)
	{
		return lhs > rhs ? rhs : lhs;
	}

	//! Compare pBeg1 with pBeg2, if pBeg1 is smaller than pBeg2, return -1, otherwise return 1.
	//! If they are the same, return 0.
	template<class T>
	int compare(const T* pBeg1, const T* pBeg2, Size n)
	{
		for (; n > 0; ++pBeg1, ++pBeg2, --n)
		{
			if (*pBeg1 != *pBeg2)
			{
				return (static_cast<typename make_unsigned<T>::type>(*pBeg1) <
					static_cast<typename make_unsigned<T>::type>(*pBeg2)) ? -1 : 1;
			}
		}
		return 0;
	}

	//! Find a value within the range [first, last)
	//! @return The first iterator i in the range [first, last) for which 
	// the following corresponding conditions hold: *i == value. Returns last if no such iterator is found.
	template <typename InputIterator, typename T>
	SG_INLINE InputIterator
	find(InputIterator first, InputIterator last, const T& value)
	{
		while ((first != last) && !(*first == value)) // Note that we always express value comparisons in terms of < or ==.
			++first;
		return first;
	}

	//! Find a subsequence within the range [first1, last1) which is identical to [first2, last2). 
	//! Complexity: O(n * m) which n is the length of [first1, last1), m is the length of [first2, last2).
	//! @return It returns an iterator pointing to the beginning of that 
	//! subsequence, or else last1 if no such subsequence exists.
	template <typename ForwardIterator1, typename ForwardIterator2>
	ForwardIterator1 search(ForwardIterator1 first1, ForwardIterator1 last1,
		ForwardIterator2 first2, ForwardIterator2 last2)
	{
		if (first2 != last2) // if there is anything to search for...
		{
			// we need to make a special case for a pattern of one element,
			// as the logic below prevents one element patterns from working.
			ForwardIterator2 temp2(first2);
			++temp2;

			if (temp2 != last2) // if what we are searching for has a length > 1...
			{
				ForwardIterator1 cur1(first1);
				ForwardIterator2 p2;

				while (first1 != last1)
				{
					// the following loop is the equivalent of eastl::find(first1, last1, *first2)
					while ((first1 != last1) && !(*first1 == *first2))
						++first1;

					if (first1 != last1)
					{
						p2 = temp2;
						cur1 = first1;

						if (++cur1 != last1)
						{
							while (*cur1 == *p2)
							{
								if (++p2 == last2)
									return first1;

								if (++cur1 == last1)
									return last1;
							}

							++first1;
							continue;
						}
					}
					return last1;
				}
				// fall through to the end.
			}
			else
				return find(first1, last1, *first2);
		}
		return first1;
	}

	template<class T>
	SG_INLINE void swap(T& lhs, T& rhs)
	{
		T temp = SG::move(lhs);
		lhs = SG::move(rhs);
		rhs = SG::move(temp);
	}

	//template <typename T>
	//inline void swap(T& a, T& b) noexcept(is_nothrow_move_constructible<T>::value && is_nothrow_move_assignable<T>::value)
	//{
	//	T temp(SG::move(a));  // EASTL_MOVE uses EASTL::move when available, else is a no-op.
	//	a = SG::move(b);
	//	b = SG::move(temp);
	//}

}

#endif // ALGORITHM_H