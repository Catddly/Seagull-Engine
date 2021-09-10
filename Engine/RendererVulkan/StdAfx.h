#pragma once

#include <eastl/set.h>

#ifdef SG_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	endif
#endif

#include <vulkan/vulkan_core.h>
