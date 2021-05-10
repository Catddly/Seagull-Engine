#pragma once

#ifdef SG_PLATFORM_WINDOWS
#	include "Platform.Windows.h"
#elif  SG_PLATFORM_MAC
#	include "Platform.Mac.h"
#elif  SG_PLATFORM_LINUX
#	include "Platform.Linux.h"
#else
#	error "Unsupported platform yet"
#endif