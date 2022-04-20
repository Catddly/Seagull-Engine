#pragma once

#include "VulkanConfig.h"

#include "volk.h"

#if SG_USE_VULKAN_MEMORY_ALLOCATOR
#	define VK_API_VERSION_MAJOR(version) (((uint32_t)(version) >> 22) & 0x7FU)
#	define VK_API_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3FFU)
#	define VK_API_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)
#	include "vma/vk_mem_alloc.h"
#endif