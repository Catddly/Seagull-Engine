#pragma once

#include <mimalloc/include/mimalloc.h>

#ifdef SG_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	endif
#	include <windowsx.h>
#	include <commdlg.h>
#endif

#include "Stl/string.h"
#include "Stl/string_view.h"
#include <EASTL/utility.h>

#include <shlwapi.h> // for win32 directory manipulation
#pragma comment(lib, "shlwapi.lib")

#include <direct.h> // _chdir()

#include <fstream>
#include <sstream>

#include <stdio.h>
#include <ctime>    // SystemTimeWindows.cpp
#include <io.h>     // _access() in FileSystem