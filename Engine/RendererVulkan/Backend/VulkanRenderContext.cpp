#include "StdAfx.h"
#include "VulkanRenderContext.h"

#include "System/ILogger.h"

namespace SG
{

	VulkanRenderContext::VulkanRenderContext(UInt32 numCommandBuffers)
	{
		commandBuffers.resize(numCommandBuffers);
	}

	void VulkanRenderContext::CmdBeginCommandBuf(VkCommandBuffer buf, bool bPermanent)
	{
		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.pNext = nullptr;
		if (bPermanent)
			cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		else
			cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if (vkBeginCommandBuffer(buf, &cmdBufInfo) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to begin command buffer!");
			SG_ASSERT(false);
		}
	}

	void VulkanRenderContext::CmdEndCommandBuf(VkCommandBuffer buf)
	{
		if (vkEndCommandBuffer(buf) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to end command buffer!");
			SG_ASSERT(false);
		}
	}

	void VulkanRenderContext::CmdSetViewport(VkCommandBuffer buf, float width, float height, float minDepth, float maxDepth)
	{
		// enable VK_KHR_Maintenance1 to filp y coordinate in screen space(viewport).
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)height;
		viewport.width  =  (float)width;
		viewport.height = -(float)height;
		viewport.minDepth = (float)minDepth;
		viewport.maxDepth = (float)maxDepth;
		vkCmdSetViewport(buf, 0, 1, &viewport);
	}

	void VulkanRenderContext::CmdSetScissor(VkCommandBuffer buf, const Rect& rect)
	{
		VkRect2D scissor = {};
		scissor.extent.width = GetRectWidth(rect);
		scissor.extent.height = GetRectHeight(rect);
		scissor.offset.x = rect.left;
		scissor.offset.y = rect.top;
		vkCmdSetScissor(buf, 0, 1, &scissor);
	}

	void VulkanRenderContext::CmdBindVertexBuffer(VkCommandBuffer buf, UInt32 firstBinding, UInt32 bindingCount, VkBuffer* buffer, const VkDeviceSize* pOffsets)
	{
		vkCmdBindVertexBuffers(buf, firstBinding, bindingCount, buffer, pOffsets);
	}

	void VulkanRenderContext::CmdBindIndexBuffer(VkCommandBuffer buf, VkBuffer buffer, UInt32 offset, VkIndexType type)
	{
		vkCmdBindIndexBuffer(buf, buffer, offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanRenderContext::CmdBindDescriptorSets(VkCommandBuffer buf, VkPipelineLayout layout, VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
	}

	void VulkanRenderContext::CmdBindPipeline(VkCommandBuffer buf, VkPipeline pipeline)
	{
		// TODO: abstract pipeline
		vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void VulkanRenderContext::CmdDraw(VkCommandBuffer buf, UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance)
	{
		vkCmdDraw(buf, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanRenderContext::CmdDrawIndexed(VkCommandBuffer buf, UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, UInt32 vertexOffset, UInt32 firstInstance)
	{
		vkCmdDrawIndexed(buf, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanRenderContext::CmdCopyBuffer(VkCommandBuffer buf, VkBuffer srcBuffer, VkBuffer dstBuffer, UInt32 sizeInByte)
	{
		VkBufferCopy copyRegion = {};
		
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = sizeInByte;

		vkCmdCopyBuffer(buf, srcBuffer, dstBuffer, 1, &copyRegion);
	}

	void VulkanRenderContext::CmdBeginRenderPass(VkCommandBuffer buf, VkRenderPass renderPass, VkFramebuffer frameBuffer, const ClearValue& clear, UInt32 width, UInt32 height)
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

		vkCmdBeginRenderPass(buf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderContext::CmdEndRenderPass(VkCommandBuffer buf)
	{
		vkCmdEndRenderPass(buf);
	}

}