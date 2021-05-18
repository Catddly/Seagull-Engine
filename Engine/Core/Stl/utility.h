#ifndef UTILITY_H
#define UTILITY_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

namespace SG
{

	template<class T>
	SG_INLINE Size len_of_char(const T* ptr)
	{
		const auto* pCurrent = ptr;
		while (pCurrent && *pCurrent)
			++pCurrent;
		return (Size)(pCurrent - ptr);
	}

}

#endif // UTILITY_H