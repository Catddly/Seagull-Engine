#include "StdAfx.h"
#include "VulkanCommand.h"

#include "Memory/Memory.h"
#include "Profile/Profile.h"

#include "VulkanConfig.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptor.h"
#include "VulkanPipelineSignature.h"
#include "VulkanPipeline.h"
#include "VulkanFrameBuffer.h"
#include "VulkanTexture.h"
#include "VulkanQueryPool.h"

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
		if (device.queueFamilyIndices.graphics == this->queueFamilyIndex)
			buffer.type = EPipelineType::eGraphic;
		else if (device.queueFamilyIndices.compute == this->queueFamilyIndex)
			buffer.type = EPipelineType::eCompute;
		else if (device.queueFamilyIndices.transfer == this->queueFamilyIndex)
			buffer.type = EPipelineType::eTransfer;
		buffer.pDevice = &device;
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
		buffer.pDevice = nullptr;
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
		return New(VulkanCommandPool, d, queueFamilyIndex, flag);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanCommandPool
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// global static variable
	static VulkanRenderPass* gpCurrRenderPass = nullptr; // used to judge whether this is a valid BeginRenderPass().
	static EPipelineType gCurrCmdType = EPipelineType::MAX_COUNT;

	void VulkanCommandBuffer::BeginRecord(bool bPermanent)
	{
		SG_PROFILE_FUNCTION();

		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.pNext = nullptr;
		if (bPermanent)
			cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		else
			cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo),
			SG_LOG_ERROR("Failed to begin command buffer!"); SG_ASSERT(false););
		gCurrCmdType = type;
	}

	void VulkanCommandBuffer::EndRecord()
	{
		SG_PROFILE_FUNCTION();

		VK_CHECK(vkEndCommandBuffer(commandBuffer),
			SG_LOG_ERROR("Failed to end command buffer!"); SG_ASSERT(false););
		gCurrCmdType = EPipelineType::MAX_COUNT;
	}

	void VulkanCommandBuffer::Reset(bool bReleaseResource)
	{
		SG_PROFILE_FUNCTION();

		vkResetCommandBuffer(commandBuffer, bReleaseResource ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
	}

	void VulkanCommandBuffer::BeginRenderPass(VulkanFrameBuffer* pFrameBuffer, const ClearValue* pClearValues, UInt32 numClearValue)
	{
		SG_PROFILE_FUNCTION();

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = pFrameBuffer->width;
		renderPassBeginInfo.renderArea.extent.height = pFrameBuffer->height;

		renderPassBeginInfo.clearValueCount = numClearValue;
		renderPassBeginInfo.pClearValues = reinterpret_cast<const VkClearValue*>(pClearValues);

		renderPassBeginInfo.renderPass = pFrameBuffer->pRenderPass->renderPass;
		renderPassBeginInfo.framebuffer = pFrameBuffer->frameBuffer;

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		gpCurrRenderPass = pFrameBuffer->pRenderPass;
	}

	void VulkanCommandBuffer::EndRenderPass()
	{
		SG_PROFILE_FUNCTION();

		for (auto& trans : gpCurrRenderPass->transitions)
		{
			if (trans.srcLayout != VK_IMAGE_LAYOUT_UNDEFINED && trans.pRenderTarget->currLayouts[0] != trans.srcLayout)
			{
				SG_LOG_WARN("Mismatch renderpass rendertarget image layout.");
				SG_ASSERT(false);
			}
			else
				trans.pRenderTarget->currLayouts[0] = trans.dstLayout;
		}
		vkCmdEndRenderPass(commandBuffer);
		gpCurrRenderPass = nullptr;
	}

	void VulkanCommandBuffer::SetViewport(float width, float height, float minDepth, float maxDepth)
	{
		SG_PROFILE_FUNCTION();

		// enable VK_KHR_Maintenance1 to flip y coordinate in screen space(viewport).
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = (float)minDepth;
		viewport.maxDepth = (float)maxDepth;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	}

	void VulkanCommandBuffer::SetScissor(const Rect& rect)
	{
		SG_PROFILE_FUNCTION();

		VkRect2D scissor = {};
		scissor.extent.width  = GetRectWidth(rect);
		scissor.extent.height = GetRectHeight(rect);
		scissor.offset.x = rect.left;
		scissor.offset.y = rect.top;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VulkanCommandBuffer::BindVertexBuffer(UInt32 firstBinding, UInt32 bindingCount, VulkanBuffer& buffer, const UInt64* pOffsets)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, &buffer.buffer, pOffsets);
	}

	void VulkanCommandBuffer::BindIndexBuffer(VulkanBuffer& buffer, UInt64 offset, VkIndexType type)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		vkCmdBindIndexBuffer(commandBuffer, buffer.buffer, offset, type);
	}

	void VulkanCommandBuffer::PushConstants(VulkanPipelineSignature* pSignature, EShaderStage shaderStage, UInt32 size, UInt32 offset, const void* pConstants)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		if (pConstants == nullptr)
		{
			SG_LOG_WARN("Can not push nullptr data!");
			return;
		}
		vkCmdPushConstants(commandBuffer, pSignature->mpPipelineLayout->layout, ToVkShaderStageFlags(shaderStage), offset, size, pConstants);
	}

	void VulkanCommandBuffer::BindDescriptorSet(VulkanPipelineSignature* pSignature, UInt32 firstSet, VulkanDescriptorSet* set, UInt32 dynamicOffsetCnt, UInt32* pOffsets, EPipelineType type)
	{
		SG_PROFILE_FUNCTION();

		VkPipelineBindPoint bp;
		switch (type)
		{
		case EPipelineType::eGraphic:
		case EPipelineType::eTransfer: bp = VK_PIPELINE_BIND_POINT_GRAPHICS; break;
		case EPipelineType::eCompute: bp = VK_PIPELINE_BIND_POINT_COMPUTE; break;
		}
		vkCmdBindDescriptorSets(commandBuffer, bp, pSignature->mpPipelineLayout->layout, firstSet, 1, &set->set, dynamicOffsetCnt, pOffsets);
	}

	void VulkanCommandBuffer::BindPipelineSignatureNonDynamic(VulkanPipelineSignature* pSignature, EPipelineType type)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		for (auto& set : pSignature->mDescriptorSetData)
		{
			VkPipelineBindPoint bp;
			switch (type)
			{
			case EPipelineType::eGraphic:
			case EPipelineType::eTransfer: bp = VK_PIPELINE_BIND_POINT_GRAPHICS; break;
			case EPipelineType::eCompute: bp = VK_PIPELINE_BIND_POINT_COMPUTE; break;
			}
			auto& descriptorSet = pSignature->GetDescriptorSet(set.first, "__non_dynamic");
			// here we assume that user know that this shader have non-dynamic buffers
			vkCmdBindDescriptorSets(commandBuffer, bp, pSignature->mpPipelineLayout->layout,
				set.first, 1, &(descriptorSet.set), 0, nullptr);
		}
	}

	void VulkanCommandBuffer::BindPipelineSignatureNonDynamic(VulkanPipelineSignature* pSignature, UInt32 set, EPipelineType type)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		VkPipelineBindPoint bp;
		switch (type)
		{
		case EPipelineType::eGraphic:
		case EPipelineType::eTransfer: bp = VK_PIPELINE_BIND_POINT_GRAPHICS; break;
		case EPipelineType::eCompute: bp = VK_PIPELINE_BIND_POINT_COMPUTE; break;
		}
		auto& descriptorSet = pSignature->GetDescriptorSet(set, "__non_dynamic");
		// here we assume that user know that this shader have non-dynamic buffers
		vkCmdBindDescriptorSets(commandBuffer, bp, pSignature->mpPipelineLayout->layout,
			set, 1, &(descriptorSet.set), 0, nullptr);
	}

	void VulkanCommandBuffer::BindPipeline(VulkanPipeline* pipeline)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		VkPipelineBindPoint bp;
		switch (pipeline->pipelineType)
		{
		case EPipelineType::eGraphic: 
		case EPipelineType::eTransfer: bp = VK_PIPELINE_BIND_POINT_GRAPHICS; break;
		case EPipelineType::eCompute: bp = VK_PIPELINE_BIND_POINT_COMPUTE; break;
		}
		vkCmdBindPipeline(commandBuffer, bp, pipeline->pipeline);
	}

	void VulkanCommandBuffer::Draw(UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, UInt32 vertexOffset, UInt32 firstInstance)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexedIndirect(VulkanBuffer* pIndirectBuffer, UInt32 offset, UInt32 dcCount, UInt32 stride)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		vkCmdDrawIndexedIndirect(commandBuffer, pIndirectBuffer->buffer, offset, dcCount, stride);
	}

	void VulkanCommandBuffer::CopyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer, UInt64 srcOffset, UInt64 dstOffset)
	{
		SG_PROFILE_FUNCTION();

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
		copyRegion.size = srcBuffer.SizeCPU();

		vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
	}

	void VulkanCommandBuffer::CopyBufferToImage(VulkanBuffer& srcBuffer, VulkanTexture& dstTexture, const vector<TextureCopyRegion>& region)
	{
		SG_PROFILE_FUNCTION();

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
			if (bufferCopyRegion.imageExtent.depth == 0)
				SG_ASSERT(false);
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

	void VulkanCommandBuffer::CopyImage(VulkanTexture& srcTexture, VulkanTexture& dstTexture, const TextureCopyRegion& region)
	{
		SG_PROFILE_FUNCTION();

		VkImageCopy copyRegion = {};

		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.layerCount = srcTexture.GetNumArray();
		copyRegion.srcOffset = { 0, 0, 0 };

		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.baseArrayLayer = region.baseArray;
		copyRegion.dstSubresource.mipLevel = region.mipLevel;
		copyRegion.dstSubresource.layerCount = region.layer;
		copyRegion.dstOffset = { 0, 0, 0 };

		copyRegion.extent.width = region.width;
		copyRegion.extent.height = region.height;
		copyRegion.extent.depth = region.depth;

		vkCmdCopyImage(commandBuffer,
			srcTexture.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstTexture.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion);
	}

	void VulkanCommandBuffer::BlitImage(VulkanTexture& srcTexture, VulkanTexture& dstTexture, const TextureBlitRegion& region, EFilterMode mode)
	{
		VkImageBlit imageBlit = {};
		imageBlit.srcOffsets[0] = { region.srcOffsets[0][0], region.srcOffsets[0][1], region.srcOffsets[0][2] };
		imageBlit.srcOffsets[1] = { region.srcOffsets[1][0], region.srcOffsets[1][1], region.srcOffsets[1][2] };
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.mipLevel = region.srcMipLevel;
		imageBlit.srcSubresource.baseArrayLayer = region.srcBaseArrayLayer;
		imageBlit.srcSubresource.layerCount = region.srcLayerCount;

		imageBlit.dstOffsets[0] = { region.dstOffsets[0][0], region.dstOffsets[0][1], region.dstOffsets[0][2] };
		imageBlit.dstOffsets[1] = { region.dstOffsets[1][0], region.dstOffsets[1][1], region.dstOffsets[1][2] };
		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.mipLevel = region.dstMipLevel;
		imageBlit.dstSubresource.baseArrayLayer = region.dstBaseArrayLayer;
		imageBlit.dstSubresource.layerCount = region.dstLayerCount;

		vkCmdBlitImage(commandBuffer, 
			srcTexture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, &imageBlit, ToVkFilterMode(mode));
	}

	void VulkanCommandBuffer::ImageBarrier(VulkanTexture* pTex, EResourceBarrier oldBarrier, EResourceBarrier newBarrier, int mipLevel)
	{
		SG_PROFILE_FUNCTION();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = ToVkImageLayout(oldBarrier);

		if (pTex) // layout transition checking
		{
			if (mipLevel == -1) // compare all
			{
				for (auto& layout : pTex->currLayouts)
				{
					if (layout != barrier.oldLayout)
					{
						SG_LOG_ERROR("Unmatched image layout transition!");
						return;
					}
				}
			}
			else
			{
				if (pTex->currLayouts[mipLevel] != barrier.oldLayout) // just compoare this mipLevel
				{
					SG_LOG_ERROR("Unmatched image layout transition!");
					return;
				}
			}
		}
		else
		{
			SG_LOG_ERROR("pTex is a nullptr!");
		}

		barrier.newLayout = ToVkImageLayout(newBarrier);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = pTex->image;
		// if the mipLevel is not -1, we know user want to transition the exact mipmap of this image.
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = (mipLevel == -1) ? 0 : static_cast<UInt32>(mipLevel);
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.levelCount = (mipLevel == -1) ? pTex->mipLevel : 1;
		barrier.subresourceRange.layerCount = pTex->array;
	
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM,
			dstStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;

		if (barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
			barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) 
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
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
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			barrier.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
			barrier.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
			barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
			barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
			barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
			barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
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
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
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

		if (mipLevel == -1)
		{
			for (auto& layout : pTex->currLayouts)
				layout = barrier.newLayout;
		}
		else
		{
			pTex->currLayouts[mipLevel] = barrier.newLayout;
		}
	}

	void VulkanCommandBuffer::BufferBarrier(VulkanBuffer* pBuf, EPipelineStageAccess oldStage, EPipelineStageAccess newStage,
		EPipelineType srcType, EPipelineType dstType)
	{
		SG_PROFILE_FUNCTION();

		if (!pBuf)
		{
			SG_LOG_ERROR("Invalid buffer!");
			return;
		}

		VkBufferMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.buffer = pBuf->buffer;
		barrier.size = pBuf->size;
		barrier.offset = 0;

		if (srcType != dstType)
		{
			if (srcType == EPipelineType::eGraphic)
				barrier.srcQueueFamilyIndex = pDevice->queueFamilyIndices.graphics;
			else if (srcType == EPipelineType::eCompute)
				barrier.srcQueueFamilyIndex = pDevice->queueFamilyIndices.compute;
			else if (srcType == EPipelineType::eTransfer)
				barrier.srcQueueFamilyIndex = pDevice->queueFamilyIndices.transfer;

			if (dstType == EPipelineType::eGraphic)
				barrier.dstQueueFamilyIndex = pDevice->queueFamilyIndices.graphics;
			else if (dstType == EPipelineType::eCompute)
				barrier.dstQueueFamilyIndex = pDevice->queueFamilyIndices.compute;
			else if (dstType == EPipelineType::eTransfer)
				barrier.dstQueueFamilyIndex = pDevice->queueFamilyIndices.transfer;
		}
		else
		{
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}

		barrier.srcAccessMask = ToVKAccessFlags(oldStage);
		barrier.dstAccessMask = ToVKAccessFlags(newStage);

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM,
			dstStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;

		if (oldStage == EPipelineStageAccess::efIndirect_Read &&
			newStage == EPipelineStageAccess::efShader_Write)
		{
			srcStage = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
			dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		else if (oldStage == EPipelineStageAccess::efShader_Write &&
			newStage == EPipelineStageAccess::efIndirect_Read)
		{
			srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
		}
		else if (oldStage == EPipelineStageAccess::efShader_Read &&
			newStage == EPipelineStageAccess::efShader_Write)
		{
			// Temporary
			srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		else if (oldStage == EPipelineStageAccess::efShader_Write &&
			newStage == EPipelineStageAccess::efShader_Read)
		{
			srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		else
		{
			SG_LOG_ERROR("Unsupported pipeline stage transition!");
			return;
		}

		vkCmdPipelineBarrier(commandBuffer,
			srcStage, dstStage, 0,
			0, nullptr,
			1, &barrier,
			0, nullptr);
 	}

	void VulkanCommandBuffer::SetDepthBias(float biasConstant, float clamp, float slopeFactor)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		vkCmdSetDepthBias(commandBuffer, biasConstant, clamp, slopeFactor);
	}

	void VulkanCommandBuffer::Dispatch(UInt32 groupX, UInt32 groupY, UInt32 groupZ)
	{
		SG_PROFILE_FUNCTION();

		if (!IsRenderPassValid())
			return;
		vkCmdDispatch(commandBuffer, groupX, groupY, groupZ);
	}

	void VulkanCommandBuffer::ResetQueryPool(VulkanQueryPool* pQueryPool)
	{
		SG_PROFILE_FUNCTION();

		if (pQueryPool->bSleep)
			return;
		vkCmdResetQueryPool(commandBuffer, pQueryPool->queryPool, 0, pQueryPool->queryCount);
	}

	void VulkanCommandBuffer::BeginQuery(VulkanQueryPool* pQueryPool, UInt32 queryIndex)
	{
		SG_PROFILE_FUNCTION();

		if (pQueryPool->bSleep)
			return;
		vkCmdBeginQuery(commandBuffer, pQueryPool->queryPool, queryIndex, 0);
	}

	void VulkanCommandBuffer::EndQuery(VulkanQueryPool* pQueryPool, UInt32 queryIndex)
	{
		SG_PROFILE_FUNCTION();

		if (pQueryPool->bSleep)
			return;
		vkCmdEndQuery(commandBuffer, pQueryPool->queryPool, queryIndex);
	}

	void VulkanCommandBuffer::WriteTimeStamp(VulkanQueryPool* pQueryPool, EPipelineStage pipelineStage, UInt32 query)
	{
		SG_PROFILE_FUNCTION();

		if (pQueryPool->bSleep)
			return;
		vkCmdWriteTimestamp(commandBuffer, ToVKPipelineStageFlags(pipelineStage), pQueryPool->queryPool, query);
	}

	bool VulkanCommandBuffer::IsRenderPassValid()
	{
		SG_PROFILE_FUNCTION();

		if (!gpCurrRenderPass && gCurrCmdType != EPipelineType::eCompute)
		{
			SG_LOG_WARN("Did you forget to call BeginRenderPass()?");
			return false;
		}
		return true;
	}

}