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

	template<class T>
	SG_CONSTEXPR SG_INLINE typename enable_if<is_scalar_v<T>, T>::type
	min(const T& lhs, const T& rhs)
	{
		return lhs < rhs ? lhs : rhs;
	}

	template<class T>
	SG_CONSTEXPR SG_INLINE typename enable_if<is_scalar_v<T>, T>::type
	max(const T& lhs, const T& rhs)
	{
		return lhs > rhs ? lhs : rhs;
	}

}

#endif // ALGORITHM_H