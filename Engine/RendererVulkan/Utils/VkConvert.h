#pragma once

#include "Render/SwapChain.h"
#include "Render/Queue.h"
#include "Render/ResourceBarriers.h"
#include "Render/Buffer.h"
#include "Render/Shader.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	VkFormat     ToVkImageFormat(EImageFormat format);
	EImageFormat ToSGImageFormat(VkFormat format);

	VkDescriptorType ToVkDescriptorType(EBufferType type);
	VkDescriptorType ToVkDescriptorType(EImageUsage usage);

	VkFormat     ToVkShaderDataFormat(EShaderDataType type);

	VkImageType  ToVkImageType(EImageType type);
	EImageType   ToSGImageType(VkImageType type);

	VkSampleCountFlagBits ToVkSampleCount(ESampleCount cnt);
	ESampleCount ToSGSampleCount(VkSampleCountFlagBits cnt);

	VkImageUsageFlags  ToVkImageUsage(EImageUsage type);
	EImageUsage ToSGImageUsage(VkImageUsageFlags usage);

	VkBufferUsageFlags ToVkBufferUsage(EBufferType type);

	VkPresentModeKHR ToVkPresentMode(EPresentMode pmode);

	VkAccessFlags ToVkAccessFlags(EResourceBarrier barrier);
	VkImageLayout ToVkImageLayout(EResourceBarrier barrier);

	VkImageViewType ToVkImageViewType(VkImageType imageType, UInt32 array);

	VkPipelineStageFlags ToVkPipelineStageFlags(VkAccessFlags accessFlags, EQueueType queueType);

}