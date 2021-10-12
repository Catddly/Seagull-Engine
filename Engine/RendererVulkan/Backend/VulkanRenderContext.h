#pragma once

#include "Platform/Window.h"
#include "Render/RenderContext.h"

#include "VulkanSwapchain.h"

#include <vulkan/vulkan_core.h>

#include "stl/vector.h"

namespace SG
{

	//! Record render commands and dispatch render jobs to render thread.
	class VulkanRenderContext : public RenderContext
	{
	public:
		vector<VkCommandBuffer> commandBuffers;
		VkCommandPool           commandPool;

		explicit VulkanRenderContext(UInt32 numCommandBuffers);
		~VulkanRenderContext() = default;

		void CmdBeginCommandBuf(VkCommandBuffer buf, bool bPermanent = false);
		void CmdEndCommandBuf(VkCommandBuffer buf);

		void CmdSetViewport(VkCommandBuffer buf, float width, float height, float minDepth, float maxDepth);
		void CmdSetScissor(VkCommandBuffer buf, const Rect& rect);

		void CmdBindVertexBuffer(VkCommandBuffer buf, UInt32 firstBinding, UInt32 bindingCount, VkBuffer* buffer, const VkDeviceSize* pOffsets);
		void CmdBindIndexBuffer(VkCommandBuffer buf, VkBuffer buffer, UInt32 offset, VkIndexType type = VK_INDEX_TYPE_UINT32);

		void CmdBindDescriptorSets(VkCommandBuffer buf, VkPipelineLayout layout, VkDescriptorSet descriptorSet);
		void CmdBindPipeline(VkCommandBuffer buf, VkPipeline pipeline);

		void CmdDraw(VkCommandBuffer buf, UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance);
		void CmdDrawIndexed(VkCommandBuffer buf, UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, UInt32 vertexOffset, UInt32 firstInstance);

		void CmdCopyBuffer(VkCommandBuffer buf, VkBuffer srcBuffer, VkBuffer dstBuffer, UInt32 sizeInByte);

		// TODO: combine it width begin render pass
		//void CmdBindRenderTarget(VulkanRenderTarget* pRt, const ClearValue& clear);
		void CmdBeginRenderPass(VkCommandBuffer buf, VkRenderPass renderPass, VkFramebuffer frameBuffer, const ClearValue& clear, UInt32 width, UInt32 height);
		void CmdEndRenderPass(VkCommandBuffer buf);
	private:
	};

}