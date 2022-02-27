#pragma once

#include "Base/BasicTypes.h"
#include "Render/Command.h"
#include "Render/Swapchain.h"
#include "Render/ResourceBarriers.h"

#include "VulkanDevice.h"

#include "volk.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanPipelineLayout;
	class VulkanPipeline;
	class VulkanFrameBuffer;
	class VulkanRenderTarget;
	class VulkanTexture;

	class VulkanCommandBuffer
	{
	public:
		void BeginRecord(bool bPermanent = false);
		void EndRecord();

		void BeginRenderPass(VulkanFrameBuffer* pFrameBuffer, const ClearValue& clear); // TODO: change param
		void EndRenderPass();

		void SetViewport(float width, float height, float minDepth, float maxDepth, bool flipY = true);
		void SetScissor(const Rect& rect);

		void BindVertexBuffer(UInt32 firstBinding, UInt32 bindingCount, VulkanBuffer& buffer, const UInt64* pOffsets);
		void BindIndexBuffer(VulkanBuffer& buffer, UInt32 offset, VkIndexType type = VK_INDEX_TYPE_UINT32);

		void PushConstants(VulkanPipelineLayout* layout, EShaderStage shaderStage, UInt32 size, UInt32 offset, void* pConstants);
		void BindDescriptorSet(VulkanPipelineLayout* pLayout, UInt32 firstSet, VkDescriptorSet descriptorSet);
		void BindPipeline(VulkanPipeline* pPipeline);

		void Draw(UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance);
		void DrawIndexed(UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, UInt32 vertexOffset, UInt32 firstInstance);

		// transfer
		void CopyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer);
		void CopyBufferToImage(VulkanBuffer& srcBuffer, VulkanTexture& dstTexture, const vector<TextureCopyRegion>& region);

		//void BufferBarrier();
		void ImageBarrier(VulkanTexture* pTex, EResourceBarrier oldBarrier, EResourceBarrier newBarrier);
		//void MemoryBarrier();
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

		bool Reset();

		const VkCommandPool&      NativeHandle() const { return commandPool; }
		static VulkanCommandPool* Create(VulkanDevice& d, VkQueueFlagBits queueType, VkCommandPoolCreateFlags flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	private:
		VulkanDevice& device;
		VkCommandPool commandPool;
		UInt32        queueFamilyIndex;
	};

}