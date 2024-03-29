#pragma once

#include "Compilers.h"
#include "Platform.h"

#ifdef _DEBUG
#	define SG_DEBUG
#else
#	define SG_RELEASE
#endif

#define SG_SEAGULL_ENGINE 1

#define SG_ENGINE_WNAME L"Seagull Engine"
#define SG_ENGINE_NAME   "Seagull Engine"

#define SG_ENGINE_VERSION_MAJOR 0
#define SG_ENGINE_VERSION_MINOR 2
#define SG_ENGINE_VERSION_PATCH 0

// helper signs
//! You can ignore this value or pointer when you use it.
#define SG_NULLABLE

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
#	define SG_COMPILE_ASSERT(x, msg) do { static_assert(x, msg); } while(false)
#endif

#ifndef SG_ASSERT
#	define SG_ASSERT(x) do { if(!(x)) __debugbreak(); } while(false)
#endif

#ifndef SG_DEPRECATED
#	define SG_DEPRECATED [[deprecated]]
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

#define SG_CLASS_NO_COPY_ASSIGNABLE(CLASS) \
	CLASS(const CLASS&) = delete; \
	CLASS& operator=(const CLASS&) = delete

#define SG_CLASS_NO_MOVE_ASSIGNABLE(CLASS) \
	CLASS(CLASS&&) = delete; \
	CLASS& operator=(CLASS&&) = delete

#define SG_CLASS_NO_COPY_MOVE_ASSIGNABLE(CLASS) \
	SG_CLASS_NO_COPY_ASSIGNABLE(CLASS); \
	SG_CLASS_NO_MOVE_ASSIGNABLE(CLASS)

#ifndef interface
#define __STRUCT__ struct 
#define interface  __STRUCT__
#endif

#define BIT(x) (1 << x)

// render relative
#define SG_ENABLE_HDR 1

#if (__cplusplus == 201402L)
#	define SG_CPP_VERSION_14 1
#elif (__cplusplus == 201703L)
#	define SG_CPP_VERSION_17 1
#elif (__cplusplus == 202002L)
#	define SG_CPP_VERSION_20 1
#endif

#if !(SG_CPP_VERSION_14 && SG_CPP_VERSION_17 && SG_CPP_VERSION_20) // default, use C++17
#	define SG_CPP_VERSION_17 1
#endif

#ifndef SG_NOT_IMPLEMENTED
#	define SG_NOT_IMPLEMENTED() SG_ASSERT(false && "This function is not implemented yet!")
#endif