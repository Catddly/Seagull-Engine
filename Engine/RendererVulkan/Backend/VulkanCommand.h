#pragma once

#include "Base/BasicTypes.h"
#include "Render/Command.h"

#include "VulkanDevice.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"

namespace SG
{

	class VulkanCommandBuffer
	{
	public:
		void BeginRecord(bool bPermanent = false);
		void EndRecord();

		void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer frameBuffer, const ClearValue& clear, UInt32 width, UInt32 height); // TODO: change param
		void EndRenderPass();

		void SetViewport(float width, float height, float minDepth, float maxDepth);
		void SetScissor(const Rect& rect);

		void BindVertexBuffer(UInt32 firstBinding, UInt32 bindingCount, VulkanBuffer& buffer, const UInt64* pOffsets);
		void BindIndexBuffer(VulkanBuffer& buffer, UInt32 offset, VkIndexType type = VK_INDEX_TYPE_UINT32);

		void BindDescriptorSet(VkPipelineLayout layout, UInt32 firstSet, VkDescriptorSet descriptorSet); // TODO: change param.
		void BindPipeline(VkPipeline pipeline); // TODO: change param.

		void Draw(UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance);
		void DrawIndexed(UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, UInt32 vertexOffset, UInt32 firstInstance);

		// transfer
		void CopyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer);
	private:
		friend class VulkanCommandPool;
		friend class VulkanQueue;

		VkCommandBuffer commandBuffer;
		UInt32          queueFamilyIndex; // which queue this command buffer should submit to.
	};

	class VulkanCommandPool
	{
	public:
		VulkanCommandPool(VulkanDevice& d, UInt32 queueFamilyIndex, VkCommandPoolCreateFlags flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		~VulkanCommandPool();

		bool AllocateCommandBuffer(VulkanCommandBuffer& buffer, bool bPrimary = true);
		void FreeCommandBuffer(VulkanCommandBuffer& buffer);

		const VkCommandPool&      NativeHandle() const { return commandPool; }
		static VulkanCommandPool* Create(VulkanDevice& d, VkQueueFlagBits queueType, VkCommandPoolCreateFlags flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	private:
		VulkanDevice& device;
		VkCommandPool commandPool;
		UInt32        queueFamilyIndex;
	};

}