#pragma once

#if defined(SG_BUILD_DLL) // If this module is a dll
#	define SG_LOG_API __declspec(dllexport)
#else
#	define SG_LOG_API
#endif

#include <cmath>
#include <xmmintrin.h>
#include <iostream>