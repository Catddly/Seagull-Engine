#pragma once

#if defined(__clang__) && !defined(_MSC_VER)
#	define SG_COMPILER_CLANG   1
#	define SG_COMPILER_VERSION (__clang_major__ * 100 + __clang_minor__)
#	define SG_COMPILER_NAME    "clang"
#elif defined(_MSC_VER)
#	define SG_COMPILER_MSVC    1
#	define SG_COMPILER_VERSION _MSC_VER
#	define SG_COMPILER_NAME    "msvc"
#	if (_MSC_VER < 1700)   // VS2010 (1600 means 2010)
#	define EA_COMPILER_MSVC_2010 1
#	define EA_COMPILER_MSVC10_0  1
#	elif (_MSC_VER < 1800) // VS2012
#	define EA_COMPILER_MSVC_2011 1 // Microsoft changed the name to VS2012 before shipping, despite referring to it as VS2011 up to just a few weeks before shipping.
#	define EA_COMPILER_MSVC11_0  1
#	define EA_COMPILER_MSVC_2012 1
#	define EA_COMPILER_MSVC12_0  1
#	elif (_MSC_VER < 1900) // VS2013
#	define EA_COMPILER_MSVC_2013 1
#	define EA_COMPILER_MSVC13_0  1
#	elif (_MSC_VER < 1910) // VS2015
#	define EA_COMPILER_MSVC_2015 1
#	define EA_COMPILER_MSVC14_0  1
#	elif (_MSC_VER < 1911) // VS2017
#	define EA_COMPILER_MSVC_2017 1
#	define EA_COMPILER_MSVC15_0  1
#	elif (_MSC_VER >= 1920) // VS2019 or higher
#	define EA_COMPILER_MSVC_2019 1
#	define EA_COMPILER_MSVC16_0  1
#	endif
#else
#	error Unknown compiler
#endif

#if defined(SG_COMPILER_MSVC)
#	include "Compiler_Windows.h"
#elif  defined(SG_COMPILER_CLANG)
#	include "Compiler_Clang.h"
#endif
