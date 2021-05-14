#pragma once

#include "Common/Core/Defs.h"

namespace SG
{

	template<class Iter1, class Iter2>
	SG_CONSTEXPR SG_INLINE bool equal(Iter1 begin1, Iter1 end1, Iter2 begin2)
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
	SG_CONSTEXPR T&& move(T&& val) noexcept
	{
		return (T&&)val;
	}

	template<class T>
	[[nodiscard]] SG_CONSTEXPR T&& forward(T& args) noexcept
	{
		return static_cast<T&&>(args);
	}

	template<class T>
	[[nodiscard]] SG_CONSTEXPR T&& forward(T&& args) noexcept
	{
		return static_cast<T&&>(args);
	}

}