#pragma once

#include "Base/BasicTypes.h"
#include "Render/Swapchain.h"
#include "Render/ResourceBarriers.h"

#include "VulkanDevice.h"

#include "volk.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanPipeline;
	class VulkanFrameBuffer;
	class VulkanRenderTarget;
	class VulkanPipelineSignature;
	class VulkanDescriptorSet;
	class VulkanTexture;

	class VulkanCommandBuffer
	{
	public:
		void BeginRecord(bool bPermanent = false);
		void EndRecord();
		void Reset(bool bReleaseResource = false);

		void BeginRenderPass(VulkanFrameBuffer* pFrameBuffer);
		void EndRenderPass();

		void SetViewport(float width, float height, float minDepth, float maxDepth);
		void SetScissor(const Rect& rect);

		void BindVertexBuffer(UInt32 firstBinding, UInt32 bindingCount, VulkanBuffer& buffer, const UInt64* pOffsets);
		void BindIndexBuffer(VulkanBuffer& buffer, UInt64 offset, VkIndexType type = VK_INDEX_TYPE_UINT32);

		void PushConstants(VulkanPipelineSignature* pSignature, EShaderStage shaderStage, UInt32 size, UInt32 offset, const void* pConstants);
		void BindDescriptorSet(VulkanPipelineSignature* pSignature, UInt32 firstSet, VulkanDescriptorSet* set, EPipelineType type = EPipelineType::eGraphic);
		void BindPipelineSignature(VulkanPipelineSignature* pSignature, EPipelineType type = EPipelineType::eGraphic);
		void BindPipeline(VulkanPipeline* pPipeline);

		void Draw(UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance);
		void DrawIndexed(UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, UInt32 vertexOffset, UInt32 firstInstance);

		void DrawIndexedIndirect(VulkanBuffer* pIndirectBuffer, UInt32 offset, UInt32 dcCount, UInt32 stride);

		// transfer
		void CopyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer, UInt64 srcOffset = 0, UInt64 dstOffset = 0);
		void CopyBufferToImage(VulkanBuffer& srcBuffer, VulkanTexture& dstTexture, const vector<TextureCopyRegion>& region);
		void CopyImage(VulkanTexture& srcTexture, VulkanTexture& dstTexture, const TextureCopyRegion& region);

		//void BufferBarrier();
		void ImageBarrier(VulkanTexture* pTex, EResourceBarrier oldBarrier, EResourceBarrier newBarrier);
		void BufferBarrier(VulkanBuffer* pBuf, EPipelineStage oldStage, EPipelineStage newStage, EPipelineType srcType = EPipelineType::eGraphic, EPipelineType dstType = EPipelineType::eGraphic);
		//void MemoryBarrier();

		void SetDepthBias(float biasConstant, float clamp, float slopeFactor);

		// compute
		void Dispatch(UInt32 groupX, UInt32 groupY, UInt32 groupZ);
	private:
		bool IsRenderPassValid();
	private:
		friend class VulkanCommandPool;
		friend class VulkanQueue;
		VulkanDevice* pDevice = nullptr;

		VkCommandBuffer commandBuffer;
		UInt32          queueFamilyIndex; // which queue this command buffer should submit to.
		EPipelineType   type;
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