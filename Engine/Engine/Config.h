#pragma once

#if defined(SG_BUILD_DLL) // If this module is a dll
#	define SG_ENGINE_API __declspec(dllexport)
#else
#	define SG_ENGINE_API __declspec(dllimport)
#endif
