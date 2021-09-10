#pragma once

#include "Render/SwapChain.h"
#include "Render/Queue.h"
#include "Render/ResourceBarriers.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	VkFormat     ToVkImageFormat(EImageFormat format);
	EImageFormat ToSGImageFormat(VkFormat format);

	VkPresentModeKHR ToVkPresentMode(EPresentMode pmode);

	VkAccessFlags ToVkAccessFlags(EResoureceBarrier barrier);

	VkImageLayout ToVkImageLayout(EResoureceBarrier barrier);

	VkPipelineStageFlags ToVkPipelineStageFlags(VkAccessFlags accessFlags, EQueueType queueType);

}