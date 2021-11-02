#include "StdAfx.h"
#include "VulkanCommand.h"

#include "Memory/Memory.h"

#include "VulkanConfig.h"
#include "VulkanBuffer.h"

namespace SG
{

	//////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanCommandPool
	//////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanCommandPool::VulkanCommandPool(VulkanDevice& d, UInt32 queueFamilyIndex, VkCommandPoolCreateFlags flag)
		:device(d)
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
		return true;
	}

	void VulkanCommandPool::FreeCommandBuffer(VulkanCommandBuffer& buffer)
	{
		vkFreeCommandBuffers(device.logicalDevice, commandPool, 1, &buffer.commandBuffer);
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

	void VulkanCommandBuffer::BeginRenderPass(VkRenderPass renderPass, VkFramebuffer frameBuffer, const ClearValue& clear, UInt32 width, UInt32 height)
	{
		VkClearValue clearValues[2];
		clearValues[0].color = { clear.color[0], clear.color[1], clear.color[2], clear.color[3], };
		clearValues[1].depthStencil = { clear.depthStencil.depth, clear.depthStencil.stencil };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;

		renderPassBeginInfo.framebuffer = frameBuffer;

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
		vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, &buffer.NativeHandle(), pOffsets);
	}

	void VulkanCommandBuffer::BindIndexBuffer(VulkanBuffer& buffer, UInt32 offset, VkIndexType type)
	{
		vkCmdBindIndexBuffer(commandBuffer, buffer.NativeHandle(), offset, type);
	}

	void VulkanCommandBuffer::BindDescriptorSet(VkPipelineLayout layout, UInt32 firstSet, VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, firstSet, 1, &descriptorSet, 0, nullptr);
	}

	void VulkanCommandBuffer::BindPipeline(VkPipeline pipeline)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
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
		copyRegion.size = srcBuffer.SizeInByte();

		vkCmdCopyBuffer(commandBuffer, srcBuffer.NativeHandle(), dstBuffer.NativeHandle(), 1, &copyRegion);
	}

}