#pragma once

#include "Render/SwapChain.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	VkFormat     ToVkImageFormat(EImageFormat format);
	EImageFormat ToSGImageFormat(VkFormat format);

	VkPresentModeKHR ToVkPresentMode(EPresentMode pmode);

}