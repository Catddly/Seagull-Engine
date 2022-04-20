#pragma once

namespace SG
{

#define VK_CHECK(EXPR, FAILEDEXPR) do { if (EXPR != VK_SUCCESS) { FAILEDEXPR } } while(false)

#ifdef SG_DEBUG
#	define SG_ENABLE_VK_VALIDATION_LAYER 1
#else
#	define SG_ENABLE_VK_VALIDATION_LAYER 0
#endif

#define SG_USE_VULKAN_MEMORY_ALLOCATOR 1

}