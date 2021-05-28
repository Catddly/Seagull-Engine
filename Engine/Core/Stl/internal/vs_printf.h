#ifndef CSPRINTF_H
#define CSPRINTF_H

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"
#include "Core/Config.h"

#ifdef SG_COMPILER_MSVC
#	pragma once
#endif

namespace SG
{

	SG_CORE_API int Vsnprintf(Char8* SG_RESTRICT pDst, Size n, const Char8* SG_RESTRICT format, va_list args);
	SG_CORE_API int Vsnprintf(Char16* SG_RESTRICT pDst, size_t n, const Char16* SG_RESTRICT format, va_list args);
	SG_CORE_API int Vsnprintf(Char32* SG_RESTRICT pDst, size_t n, const Char32* SG_RESTRICT format, va_list args);
	SG_CORE_API int Vsnprintf(WChar* SG_RESTRICT pDst, size_t n, const WChar* SG_RESTRICT format, va_list args);

}

#endif