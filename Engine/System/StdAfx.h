#pragma once

#ifdef SG_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	endif
#endif

#include <stdio.h>
#include <stdarg.h>

#include <time.h>