#pragma once

#include "Render/SwapChain.h"
#include "Render/Queue.h"
#include "Render/ResourceBarriers.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	VkFormat     ToVkImageFormat(EImageFormat format);
	EImageFormat ToSGImageFormat(VkFormat format);

	VkImageType  ToVkImageType(EImageType type);
	EImageType   ToSGImageType(VkImageType type);

	VkSampleCountFlagBits ToVkSampleCount(ESampleCount cnt);
	ESampleCount ToSGSampleCount(VkSampleCountFlagBits cnt);

	VkImageUsageFlags  ToVkImageUsage(ERenderTargetUsage usage);
	ERenderTargetUsage ToSGImageUsage(VkImageUsageFlags usage);

	VkPresentModeKHR ToVkPresentMode(EPresentMode pmode);

	VkAccessFlags ToVkAccessFlags(EResoureceBarrier barrier);

	VkImageLayout ToVkImageLayout(EResoureceBarrier barrier);

	VkImageViewType ToVkImageViewType(VkImageType imageType, UInt32 array);

	VkPipelineStageFlags ToVkPipelineStageFlags(VkAccessFlags accessFlags, EQueueType queueType);

}