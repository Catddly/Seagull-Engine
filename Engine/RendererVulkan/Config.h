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

#define SG_USE_PACKED_BUFFER 1

//! When compiling shaders, should we use vulkan sdk?
#define SG_FORCE_USE_VULKAN_SDK 0

#define SG_ENABLE_GPU_CULLING 1