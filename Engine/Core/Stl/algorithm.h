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

}

#endif // ALGORITHM_H