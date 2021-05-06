#pragma once

#if defined(SG_BUILD_DLL) // If this module is a dll
#	define SG_COMMON_API __declspec(dllexport)
#else
#	define SG_COMMON_API
#endif

#include <cmath>
#include <xmmintrin.h>