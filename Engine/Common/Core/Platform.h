#pragma once

#ifdef SG_PLATFORM_WINDOWS
#	include "Platform_Windows.h"
#elif  SG_PLATFORM_MAC
#	include "Platform_Mac.h"
#elif  SG_PLATFORM_LINUX
#	include "Platform_Linux.h"
#else
#	error "Unsupported platform"
#endif