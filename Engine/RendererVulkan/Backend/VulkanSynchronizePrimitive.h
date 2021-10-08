#pragma once

#include "Render/Synchronize.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct VulkanSemaphore : public RenderSemaphore
	{
		VkSemaphore semaphore;
	};

	struct VulkanFence : public RenderFence
	{
		VkFence fence;
	};

}