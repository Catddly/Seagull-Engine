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

#define SG_FORCE_USE_VULKAN_SDK 0

#define SG_ENABLE_GPU_CULLING 0