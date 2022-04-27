#pragma once

#include "Base/BasicTypes.h"
#include "System/Logger.h"

#include "VulkanSynchronizePrimitive.h"
#include "VulkanBuffer.h"

namespace SG
{

	class VulkanQueue
	{
	public:
		template <Size waitStageCount, Size SignalSemaphoreCount, Size waitSemaphoreCount>
		bool SubmitCommands(VulkanCommandBuffer* pCmdBuf, EPipelineStage* waitStages,
			VulkanSemaphore** ppSignalSemaphores, VulkanSemaphore** ppWaitSemaphores,
			VulkanFence* fence);
		void WaitIdle() const;
	private:
		friend class VulkanSwapchain;
		friend class VulkanDevice;

		UInt32         familyIndex;
		EQueueType     type = EQueueType::eNull;
		EQueuePriority priority = EQueuePriority::eNormal;
		VkQueue        handle = VK_NULL_HANDLE;
	};

	template <Size waitStageCount, Size SignalSemaphoreCount, Size waitSemaphoreCount>
	bool VulkanQueue::SubmitCommands(VulkanCommandBuffer* pCmdBuf, EPipelineStage* waitStages,
		VulkanSemaphore** ppSignalSemaphores, VulkanSemaphore** ppWaitSemaphores,
		VulkanFence* fence)
	{
		if (pCmdBuf->queueFamilyIndex != familyIndex) // check if the command is submitted to the right queue
		{
			SG_LOG_ERROR("Vulkan command buffer had been submit to the wrong queue! (Submit To: %s)",
				(type == EQueueType::eGraphic) ? "Graphic" : (type == EQueueType::eCompute ? "Compute" : "Transfer"));
			return false;
		}

		SG_COMPILE_ASSERT(waitStageCount == waitSemaphoreCount, "Every wait semaphores must have a corresponding wait stage");

		// pipeline stage at which the queue submission will wait (via pWaitSemaphores)
		// the submit info structure specifies a command buffer queue submission batch
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		if constexpr (waitStageCount != 0)
		{
			VkPipelineStageFlags waitStageMask[waitStageCount] = {};
			for (Size i = 0; i < waitStageCount; ++i)
				waitStageMask[i] = ToVKPipelineStageFlags(waitStages[i]);
			//waitStageMask[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			//waitStageMask[1] = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			submitInfo.pWaitDstStageMask = waitStageMask;
		}

		if constexpr (SignalSemaphoreCount != 0)
		{
			VkSemaphore signalSemaphores[SignalSemaphoreCount] = {};
			for (Size i = 0; i < SignalSemaphoreCount; ++i)
				signalSemaphores[i] = ppSignalSemaphores[i]->semaphore;
			submitInfo.pSignalSemaphores = signalSemaphores;
			submitInfo.signalSemaphoreCount = SignalSemaphoreCount;
		}
		else
		{
			submitInfo.pSignalSemaphores = nullptr;
			submitInfo.signalSemaphoreCount = 0;
		}

		if constexpr (waitSemaphoreCount != 0)
		{
			VkSemaphore waitSemaphores[waitSemaphoreCount] = {};
			for (Size i = 0; i < waitSemaphoreCount; ++i)
				waitSemaphores[i] = ppWaitSemaphores[i]->semaphore;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.waitSemaphoreCount = waitSemaphoreCount;
		}
		else
		{
			submitInfo.pWaitSemaphores = nullptr;
			submitInfo.waitSemaphoreCount = 0;
		}

		submitInfo.pCommandBuffers = &pCmdBuf->commandBuffer;
		submitInfo.commandBufferCount = 1;

		VK_CHECK(vkQueueSubmit(handle, 1, &submitInfo, fence ? fence->fence : VK_NULL_HANDLE),
			SG_LOG_ERROR("Failed to submit render commands to queue!"); return false;);
		return true;
	}

}