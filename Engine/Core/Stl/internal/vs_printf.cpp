#include "StdAfx.h"
#include "vs_printf.h"

#ifndef SG_USER_CUSTOM_VS_PRINTF // use c sprintf
#	include <cwchar>
#endif

namespace SG
{

#ifndef SG_USER_CUSTOM_VS_PRINTF // use c sprintf
	int Vsnprintf(Char8* SG_RESTRICT pDst, Size n, const Char8* SG_RESTRICT format, va_list args)
	{
		return vsnprintf(pDst, n, format, args);
	}
	int Vsnprintf(Char16* SG_RESTRICT pDst, size_t n, const Char16* SG_RESTRICT format, va_list args)
	{
		return vswprintf((WChar*)pDst, n, (WChar*)format, args);
	}
	int Vsnprintf(Char32* SG_RESTRICT pDst, size_t n, const Char32* SG_RESTRICT format, va_list args)
	{
		return vswprintf((WChar*)pDst, n, (WChar*)format, args);
	}
	int Vsnprintf(WChar* SG_RESTRICT pDst, size_t n, const WChar* SG_RESTRICT format, va_list args)
	{
		return vswprintf(pDst, n, format, args);
	}
#endif

}