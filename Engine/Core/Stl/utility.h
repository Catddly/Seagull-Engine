#ifndef UTILITY_H
#define UTILITY_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

#include <cstring>

namespace SG
{

	template<class T>
	SG_INLINE Size len_of_char_str(const T* ptr)
	{
		const auto* pCurrent = ptr;
		while (pCurrent && *pCurrent)
			++pCurrent;
		return (Size)(pCurrent - ptr);
	}

	SG_INLINE Char8* assign_char_n(Char8* pDst, Size n, const Char8 c)
	{
		if (n != 0)
			memset(pDst, (UInt8)c, (Size)n);
		return pDst + n;
	}

	template<class T>
	SG_INLINE T* assign_char_n(T* pDst, Size n, T c)
	{
		const T* const pEnd = pDst + n;
		while (pDst < pEnd)
			*(pDst++) = c;
		return pDst + n;
	}

}

#endif // UTILITY_H