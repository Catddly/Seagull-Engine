#ifndef ALGORITHM_H
#define ALGORITHM_H

#pragma once

#include "Common/Core/Defs.h"
#include "type_traits.h"

namespace SG
{

	template<class Iter1, class Iter2>
	SG_CONSTEXPR SG_INLINE bool 
	equal(Iter1 begin1, Iter1 end1, Iter2 begin2)
	{
		while (true)
		{
			if (*begin1 != *begin2)
				return false;
			++begin1; ++begin2;
			if (begin1 == end1)
				break;
		}
		return true;
	}

	//template<class T>
	//SG_CONSTEXPR SG_INLINE T min(T lhs, T rhs)
	//{
	//	return lhs < rhs ? lhs : rhs;
	//}

	//template<class T>
	//SG_CONSTEXPR SG_INLINE T max(T lhs, T rhs)
	//{
	//	return lhs > rhs ? lhs : rhs;
	//}

	template <typename T>
	inline SG_CONSTEXPR typename enable_if<is_scalar<T>::value, T>::type
		max_alt(T a, T b)
	{
		return a < b ? b : a;
	}

	template <typename T>
	inline typename enable_if<!is_scalar<T>::value, const T&>::type
		max_alt(const T& a, const T& b)
	{
		return a < b ? b : a;
	}

	template <typename T>
	inline SG_CONSTEXPR typename enable_if<is_scalar<T>::value, T>::type
		min_alt(T a, T b)
	{
		return a > b ? b : a;
	}

	template <typename T>
	inline typename enable_if<!is_scalar<T>::value, const T&>::type
		min_alt(const T& a, const T& b)
	{
		return a > b ? b : a;
	}

}

#endif // ALGORITHM_H