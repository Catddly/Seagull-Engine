#pragma once

#define SG_PLATFORM_NAME "Windows"

#ifdef _WIN64
#	define SG_PLATFORM_WIN64 1
#else
#	define SG_PLATFORM_WIN32 1
#endif

#if defined(_M_AMD64) || defined(_AMD64_) || defined(__x86_64__)
#	define SG_PROCESSOR_X64 1
#	define SG_LITTLE_ENDIAN 1
#elif defined(_M_IX86) || defined(_X86_)
#	define SG_PROCESSOR_X86 1
#	define EA_LITTLE_ENDIAN 1
#elif defined(_M_IA64) || defined(_IA64_)
#	define SG_PROCESSOR_IA64 1
#	define EA_LITTLE_ENDIAN 1
#elif defined(_M_ARM)
#	define SG_PROCESSOR_ARM 1
#	define EA_LITTLE_ENDIAN 1
#else
#	error Unknown processor
#	error Unknown endianness
#endif