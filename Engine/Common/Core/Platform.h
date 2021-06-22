#pragma once

#if defined(__x86_64__) || defined(_M_X64)
#	define SG_PLATFORM_X64   1
#	define SG_PLATFORM_SSE2  1
#elif defined(__i386) || defined(_M_IX86) || defined(__arm__)
#	error 32-bit platforms are not supported.
#elif defined(__aarch64__)
#	define SG_PLATFORM_ARM  1
#else
#	error Unknown CPU
#endif

#ifdef SG_PLATFORM_WINDOWS
#	include "Platform_Windows.h"
#elif  SG_PLATFORM_MAC
#	include "Platform_Mac.h"
#elif  SG_PLATFORM_LINUX
#	include "Platform_Linux.h"
#else
#	error Unsupported platform
#endif