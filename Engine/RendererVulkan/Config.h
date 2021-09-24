#pragma once

#if defined(SG_BUILD_DLL) // If this module is a dll
#	if defined(SG_MODULE)
#		define SG_RENDERER_VK_API __declspec(dllexport)
#	else
#		define SG_RENDERER_VK_API __declspec(dllimport)
#	endif
#else
#	define SG_RENDERER_VK_API
#endif