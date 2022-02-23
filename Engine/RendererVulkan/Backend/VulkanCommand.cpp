#include "StdAfx.h"
#include "VulkanCommand.h"

#include "Memory/Memory.h"

#include "VulkanConfig.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanFrameBuffer.h"
#include "VulkanSwapchain.h"

namespace SG
{

	//////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanCommandPool
	//////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanCommandPool::VulkanCommandPool(VulkanDevice& d, UInt32 qFamilyIndex, VkCommandPoolCreateFlags flag)
		:device(d), queueFamilyIndex(qFamilyIndex)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = flag;
		VK_CHECK(vkCreateCommandPool(d.logicalDevice, &cmdPoolInfo, nullptr, &commandPool),
			SG_LOG_ERROR("Failed to create vulkan command pool!"););
	}

	VulkanCommandPool::~VulkanCommandPool()
	{
		vkDestroyCommandPool(device.logicalDevice, commandPool, nullptr);
	}

	bool VulkanCommandPool::AllocateCommandBuffer(VulkanCommandBuffer& buffer, bool bPrimary)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.level = bPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		commandBufferAllocateInfo.commandBufferCount = 1;

		VK_CHECK(vkAllocateCommandBuffers(device.logicalDevice, &commandBufferAllocateInfo, &buffer.commandBuffer),
			SG_LOG_ERROR("Failed to allocate command buffer"); return false;);

		buffer.queueFamilyIndex = this->queueFamilyIndex;
		return true;
	}

	void VulkanCommandPool::FreeCommandBuffer(VulkanCommandBuffer& buffer)
	{
		if (buffer.queueFamilyIndex == queueFamilyIndex)
			vkFreeCommandBuffers(device.logicalDevice, commandPool, 1, &buffer.commandBuffer);
		else
		{
			SG_LOG_ERROR("Free wrong command buffer inside the wrong pool!");
			return;
		}
	}

	bool VulkanCommandPool::Reset()
	{
		VkResult res = vkResetCommandPool(device.logicalDevice, commandPool, 0);
		if (res == 0)
			return true;
		else
		{
			SG_LOG_ERROR("[Vulkan] Error: VkResult = %d. VulkanCommandPool::Reset()", res);
			return false;
		}
	}

	VulkanCommandPool* VulkanCommandPool::Create(VulkanDevice& d, VkQueueFlagBits queueType, VkCommandPoolCreateFlags flag)
	{
		UInt32 queueFamilyIndex = d.queueFamilyIndices.graphics;
		switch (queueType)
		{
		case VK_QUEUE_GRAPHICS_BIT: break;
		case VK_QUEUE_COMPUTE_BIT:  queueFamilyIndex = d.queueFamilyIndices.compute;  break;
		case VK_QUEUE_TRANSFER_BIT: queueFamilyIndex = d.queueFamilyIndices.transfer; break;
		case VK_QUEUE_SPARSE_BINDING_BIT:
		case VK_QUEUE_PROTECTED_BIT:
		case VK_QUEUE_FLAG_BITS_MAX_ENUM:
		default: SG_LOG_WARN("Unsupported queue type!"); break;
		}
		return Memory::New<VulkanCommandPool>(d, queueFamilyIndex, flag);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanCommandPool
	//////////////////////////////////////////////////////////////////////////////////////////////////

	void VulkanCommandBuffer::BeginRecord(bool bPermanent)
	{
		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.pNext = nullptr;
		if (bPermanent)
			cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		else
			cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo),
			SG_LOG_ERROR("Failed to begin command buffer!"); SG_ASSERT(false););
	}

	void VulkanCommandBuffer::EndRecord()
	{
		VK_CHECK(vkEndCommandBuffer(commandBuffer),
			SG_LOG_ERROR("Failed to end command buffer!"); SG_ASSERT(false););
	}

	void VulkanCommandBuffer::BeginRenderPass(VulkanFrameBuffer* pFrameBuffer, const ClearValue& clear)
	{
		VkClearValue clearValues[2];
		clearValues[0].color = { clear.color[0], clear.color[1], clear.color[2], clear.color[3] };
		clearValues[1].depthStencil = { clear.depthStencil.depth, clear.depthStencil.stencil };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width  = pFrameBuffer->width;
		renderPassBeginInfo.renderArea.extent.height = pFrameBuffer->height;

		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues    = clearValues;

		renderPassBeginInfo.renderPass = pFrameBuffer->currRenderPass;
		renderPassBeginInfo.framebuffer = pFrameBuffer->frameBuffer;

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanCommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void VulkanCommandBuffer::SetViewport(float width, float height, float minDepth, float maxDepth)
	{
		// enable VK_KHR_Maintenance1 to flip y coordinate in screen space(viewport).
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)height;
		viewport.width = (float)width;
		viewport.height = -(float)height;
		viewport.minDepth = (float)minDepth;
		viewport.maxDepth = (float)maxDepth;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	}

	void VulkanCommandBuffer::SetScissor(const Rect& rect)
	{
		VkRect2D scissor = {};
		scissor.extent.width  = GetRectWidth(rect);
		scissor.extent.height = GetRectHeight(rect);
		scissor.offset.x = rect.left;
		scissor.offset.y = rect.top;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VulkanCommandBuffer::BindVertexBuffer(UInt32 firstBinding, UInt32 bindingCount, VulkanBuffer& buffer, const UInt64* pOffsets)
	{
		vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, &buffer.buffer, pOffsets);
	}

	void VulkanCommandBuffer::BindIndexBuffer(VulkanBuffer& buffer, UInt32 offset, VkIndexType type)
	{
		vkCmdBindIndexBuffer(commandBuffer, buffer.buffer, offset, type);
	}

	void VulkanCommandBuffer::PushConstants(VulkanPipelineLayout* layout, EShaderStage shaderStage, UInt32 size, UInt32 offset, void* pConstants)
	{
		if (pConstants == nullptr)
		{
			SG_LOG_WARN("Can not push nullptr data!");
			return;
		}
		vkCmdPushConstants(commandBuffer, layout->layout, ToVkShaderStageFlags(shaderStage), offset, size, pConstants);
	}

	void VulkanCommandBuffer::BindDescriptorSet(VulkanPipelineLayout* layout, UInt32 firstSet, VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout->layout, firstSet, 1, &descriptorSet, 0, nullptr);
	}

	void VulkanCommandBuffer::BindPipeline(VulkanPipeline* pipeline)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
	}

	void VulkanCommandBuffer::Draw(UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance)
	{
		vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, UInt32 vertexOffset, UInt32 firstInstance)
	{
		vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::CopyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer)
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = srcBuffer.SizeInByteCPU();

		vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
	}

	void VulkanCommandBuffer::CopyBufferToImage(VulkanBuffer& srcBuffer, VulkanTexture& dstTexture, const vector<TextureCopyRegion>& region)
	{
		vector<VkBufferImageCopy> bufferCopyRegions;
		bufferCopyRegions.resize(region.size());
		for (UInt32 i = 0; i < region.size(); ++i)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = region[i].mipLevel;
			bufferCopyRegion.imageSubresource.baseArrayLayer = region[i].baseArray;
			bufferCopyRegion.imageSubresource.layerCount = region[i].layer;
			bufferCopyRegion.imageExtent.width = region[i].width;
			bufferCopyRegion.imageExtent.height = region[i].height;
			bufferCopyRegion.imageExtent.depth = region[i].depth;
			bufferCopyRegion.bufferOffset = region[i].offset;
			bufferCopyRegions[i] = eastl::move(bufferCopyRegion);
		}

		vkCmdCopyBufferToImage(commandBuffer,
			srcBuffer.buffer,
			dstTexture.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<UInt32>(bufferCopyRegions.size()),
			bufferCopyRegions.data());
	}

	void VulkanCommandBuffer::ImageBarrier(VulkanTexture* pTex, EResourceBarrier oldBarrier, EResourceBarrier newBarrier)
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = ToVkImageLayout(oldBarrier);

		if (pTex->currLayout != barrier.oldLayout) // layout transition checking
		{
			SG_LOG_ERROR("Unmatched image layout transition!");
			return;
		}

		barrier.newLayout = ToVkImageLayout(newBarrier);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = pTex->image;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = pTex->mipLevel;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
	
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM,
			dstStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;

		if (barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			//if (hasStencilComponent(format)) {
			//	barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			//}
		}

		if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
			barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else 
		{
			SG_LOG_ERROR("Unsupported resource transition!");
			return;
		}

		vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		pTex->currLayout = barrier.newLayout;
	}

}