#pragma once

#include "Compilers.h"
#include "Platform.h"

#ifdef _DEBUG
#	define SG_DEBUG
#else
#	define SG_RELEASE
#endif

#define SG_ENGINE_WNAME L"Seagull Engine"
#define SG_ENGINE_NAME   "Seagull Engine"

#define SG_ENGINE_VERSION_MAJOR 0
#define SG_ENGINE_VERSION_MINOR 2
#define SG_ENGINE_VERSION_PATCH 0

// TODO: intelligence
#define SG_GRAPHICS_API_VULKAN 1

#if   defined(SG_PLATFORM_X64)
#	define SG_PTR_SIZE 8
#else
#	define SG_PTR_SIZE 4
#endif

#ifdef SG_CUSTOM_CONFIG_CPP_VERSION
#	if __cplusplus >= 201703L
#		define SG_CXX_17
#	else
#		error "Seagull Engine needs C++17 to compile"
#	endif
#endif

#ifndef SG_DEBUG
#	define SG_INLINE inline
#else
#	define SG_INLINE SG_FORCE_INLINE
#endif

#ifndef SG_COMPILE_ASSERT
#	define SG_COMPILE_ASSERT(x, MSG) do { static_assert(x, #MSG); } while(false)
#endif

#ifndef SG_ASSERT
#	define SG_ASSERT(x) do { if(!(x)) __debugbreak(); } while(false)
#endif

#define SG_NO_USE(x) (void(x))

#define SG_CONSTEXPR constexpr

#define SG_THREAD_LOCAL __declspec(thread)

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

// maybe move to another file 
#define SG_CLASS_NO_COPY_ASSIGNABLE(CLASS) CLASS(const CLASS&) = delete; \
	CLASS operator=(const CLASS&) = delete

#ifndef interface
#define __STRUCT__ struct 
#define interface  __STRUCT__
#endif