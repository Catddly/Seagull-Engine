#pragma once

#include <stdint.h>

#if INTPTR_MAX == 0x7FFFFFFFFFFFFFFFLL
#	define SPTR_SIZE 8
#else INTPTR_MAX == 0x7FFFFFFF
#	define SPTR_SIZE 4
#endif

#ifdef SG_CUSTOM_CONFIG_CPP_VERSION
#	if __cplusplus >= 201703L
#		define SG_CXX_17
#	else
#		error "Seagull Engine needs C++17 to compile"
#	endif
#endif

#ifdef __cplusplus
#	define SG_FORCE_INLINE __forceinline
#else
#	define SG_FORCE_INLINE
#endif

#ifndef SG_COMPILE_ASSERT
#	define SG_COMPILE_ASSERT(x) static_assert(x)
#endif