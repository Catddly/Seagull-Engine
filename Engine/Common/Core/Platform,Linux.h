#pragma once

#define SG_PLATFORM_UNIX
#define SG_PLATFORM_POSIX
#define SG_PLATFORM_NAME "Linux"

#if defined(__i386__) || defined(__intel__) || defined(_M_IX86)
#	define SG_PROCESSOR_X86 1
#	define SG_LITTLE_ENDIAN 1
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_SGBI__)
#	define SG_PROCESSOR_ARM32 1
#elif defined(__aarch64__) || defined(__AARCH64)
#	define SG_PROCESSOR_ARM64 1
#elif defined(__x86_64__)
#	define SG_PROCESSOR_X86_64 1
#	define SG_SYSTEM_LITTLE_ENDIAN 1
#elif defined(__powerpc64__)
#	define SG_PROCESSOR_POWERPC 1
#	define SG_PROCESSOR_POWERPC_64 1
#	define SG_BIG_ENDIAN 1
#elif defined(__powerpc__)
#	define SG_PROCESSOR_POWERPC 1
#	define SG_PROCESSOR_POWERPC_32 1
#	define SG_BIG_ENDIAN 1
#else
#	error Unknown processor
#	error Unknown endianness
#endif