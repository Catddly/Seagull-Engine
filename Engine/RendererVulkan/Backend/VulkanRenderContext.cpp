#include "StdAfx.h"
#include "VulkanRenderContext.h"

#include "System/ILogger.h"

namespace SG
{

	VulkanRenderContext::VulkanRenderContext(UInt32 numCommandBuffers)
	{
		commandBuffers.resize(numCommandBuffers);
		frameBuffers.resize(numCommandBuffers);
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
		VkViewport viewport = {};
		viewport.height = (float)height;
		viewport.width = (float)width;
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

	void VulkanRenderContext::CmdBindPipeline(VkCommandBuffer buf, VkPipeline pipeline)
	{
		// TODO: abstract pipeline
		vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void VulkanRenderContext::CmdDraw(VkCommandBuffer buf, UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance)
	{
		vkCmdDraw(buf, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanRenderContext::CmdBeginRenderPass(VkCommandBuffer buf, VkRenderPass renderPass, VkFramebuffer frameBuffer, const ClearValue & clear, UInt32 width, UInt32 height)
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