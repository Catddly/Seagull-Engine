#pragma once

#if defined(SG_BUILD_DLL) // If this module is a dll
#	define SG_RENDERER_VK_API __declspec(dllexport)
#else
#	define SG_RENDERER_VK_API
#endif