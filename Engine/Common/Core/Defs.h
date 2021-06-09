#pragma once

#include "Compilers.h"
#include <stdint.h>

#define SG_ENGINE_WNAME L"Seagull Engine"
#define SG_ENGINE_NAME   "Seagull Engine"

#if   INTPTR_MAX == 0x7FFFFFFFFFFFFFFFLL
#	define SG_PTR_SIZE 8
#else INTPTR_MAX == 0x7FFFFFFF
#	define SG_PTR_SIZE 4
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

#define SG_INLINE inline

#ifdef __cplusplus
#	define SG_ALIGN(x) alignas(x)
#else
#	define SG_ALIGN(x)
#endif

#ifndef SG_COMPILE_ASSERT
#	define SG_COMPILE_ASSERT(x) static_assert((x))
#endif

#ifndef SG_ASSERT
#	define SG_ASSERT(x) do { if(!(x)) __debugbreak(); } while(false)
#endif

#define SG_NO_USE(x) (void(x))

#define SG_CONSTEXPR constexpr

#ifdef SG_ALLOW_EXCEPTION
#	define SG_ENABLE_EXCEPTION 1
#else
#	define SG_ENABLE_EXCEPTION 0
#endif

#ifndef SG_RESTRICT
#	if defined(SG_COMPILER_MSVC) && (SG_COMPILER_VERSION >= 1400)
#		define SG_RESTRICT __restrict
#	elif defined(SG_COMPILER_CLANG)
#		define SG_RESTRICT __restrict
#	else
#		define SG_RESTRICT
#	endif
#endif

#ifdef SG_COMPILER_MSVC
#	define SG_DISABLE_MSVC_WARNING(CODE) \
		__pragma(warning(push))          \
		__pragma(warning(disable:CODE))
#else
#	define SG_DISABLE_MSVC_WARNING(CODE)
#endif

#ifdef SG_COMPILER_MSVC
#	define SG_RESTORE_MSVC_WARNING() \
		__pragma(warning(pop))
#else
#	define SG_RESTORE_MSVC_WARNING()
#endif