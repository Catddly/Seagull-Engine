#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#include <shlwapi.h> // for win32 directory manipulation
#pragma comment(lib, "shlwapi.lib")

#include <direct.h> // _chdir()

#include <stdio.h>
#include <ctime>    // SystemTimeWindows.cpp