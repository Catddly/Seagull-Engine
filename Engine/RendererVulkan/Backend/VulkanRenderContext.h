#pragma once

#include "Platform/Window.h"
#include "Render/CommandBuffers.h"

#include "VulkanSwapchain.h"

#include <vulkan/vulkan_core.h>

#include "stl/vector.h"

namespace SG
{

	//! Record render commands and dispatch render jobs to render thread.
	class VulkanRenderContext
	{
	public:
		vector<VkCommandBuffer> commandBuffers;

		explicit VulkanRenderContext(UInt32 numCommandBuffers);
		~VulkanRenderContext() = default;

		void CmdBeginCommandBuf(VkCommandBuffer buf);
		void CmdEndCommandBuf(VkCommandBuffer buf);

		void CmdSetViewport(VkCommandBuffer buf, float width, float height, float minDepth, float maxDepth);
		void CmdSetScissor(VkCommandBuffer buf, const Rect& rect);

		void CmdBindPipeline(VkCommandBuffer buf, VkPipeline pipeline);

		void CmdDraw(VkCommandBuffer buf, UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance);

		// TODO: combine it width begin render pass
		//void CmdBindRenderTarget(VulkanRenderTarget* pRt, const ClearValue& clear);
	private:
		void CmdBeginRenderPass(VkCommandBuffer buf, VkRenderPass renderPass, const ClearValue& clear, UInt32 width, UInt32 height);
		void CmdEndRenderPass(VkCommandBuffer buf);
	};

}