#include "StdAfx.h"
#include "VulkanAllocator.h"

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
#	define VMA_IMPLEMENTATION
#	include "vma/vk_mem_alloc.h"
#endif