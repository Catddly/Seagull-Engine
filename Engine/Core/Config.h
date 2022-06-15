#pragma once

#if defined(SG_BUILD_DLL) // If this module is a dll
#	if defined(SG_MODULE)
#		define SG_CORE_API __declspec(dllexport)
#	else
#		define SG_CORE_API __declspec(dllimport)
#	endif
#else
#	define SG_CORE_API
#endif

#define SG_PROJ_FLIP_Y 1

#define SG_ENABLE_PROFILE 0

#define SG_ENABLE_MEMORY_TRACKING 0
#define SG_ENABLE_MEMORY_LEAK_DETECTION 0
#define SG_ENABLE_MEMORY_PROFILE 0
#define SG_USE_DEFAULT_MEMORY_ALLOCATION 1
