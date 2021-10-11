#pragma once

#include "RendererVulkan/Config.h"
#include "Base/BasicTypes.h"
#include "Render/Queue.h"
#include "Render/Shader.h"
#include "Render/Pipeline.h"
#include "Render/SwapChain.h"
#include "Render/Buffer.h"

#include "RendererVulkan/Backend/VulkanBuffer.h"

#include <vulkan/vulkan_core.h>

#include <Stl/vector.h>
#include <Stl/string.h>

namespace SG
{

	struct VulkanPipeline : public Pipeline
	{
		VkPipeline      pipeline;
		VkPipelineCache pipelineCache;
		VkRenderPass    renderPass;
	};

	struct VulkanQueue : public Queue
	{
		UInt32         familyIndex;
		EQueueType     type = EQueueType::eNull;
		EQueuePriority priority = EQueuePriority::eNormal;
		VkQueue        handle = VK_NULL_HANDLE;

		virtual bool SubmitCommands(RenderContext* pContext, UInt32 bufferIndex, RenderSemaphore* renderSemaphore, RenderSemaphore* presentSemaphore, RenderFence* fence) override;
		virtual void WaitIdle() const override;
	};

	struct VulkanRenderTarget;
	class  VulkanRenderContext;

	//! @brief All the device relative data are stored in here.
	//! Should be initialized by VulkanInstace.
	class VulkanDevice
	{
	public:
		VulkanDevice(VkPhysicalDevice device);
		~VulkanDevice();

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice         logicalDevice = VK_NULL_HANDLE;

		VkCommandPool    graphicCommandPool;
		VkCommandPool    computeCommandPool;
		VkCommandPool    transferCommandPool;
		VkRenderPass     defaultRenderPass;

		vector<VkExtensionProperties>   supportedExtensions;
		vector<VkQueueFamilyProperties> queueFamilyProperties;
		struct
		{
			UInt32 graphics;
			UInt32 compute;
			UInt32 transfer;
		} queueFamilyIndices;

		void WaitIdle() const;

		//! @brief Fetch all the queue family indices and create a logical device.
		bool CreateLogicalDevice(void* pNext);
		void DestroyLogicalDevice();

		//! @brief Create a command pool.
		VkCommandPool CreateCommandPool(UInt32 queueFamilyIndices, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		void DestroyCommandPool(VkCommandPool pool);

		VkSemaphore CreateSemaphore();
		void        DestroySemaphore(VkSemaphore semaphore);
		VkFence     CreateFence(bool bSignaled = false);
		void        DestroyFence(VkFence fence);

		void        ResetFence(VkFence fence);

		bool AllocateCommandBuffers(VulkanRenderContext* pContext);
		void FreeCommandBuffers(VulkanRenderContext* pContext);

		VulkanRenderContext* CreateRenderContext(UInt32 numBuffers, EQueueType type);
		void                 DestroyRenderContext(VulkanRenderContext* pContext);

		VkFramebuffer CreateFrameBuffer(VkRenderPass renderPass, VulkanRenderTarget* pColorRt, VulkanRenderTarget* pDepthRt);
		void DestroyFrameBuffer(VkFramebuffer frameBuffer);

		VulkanRenderTarget* CreateRenderTarget(const RenderTargetCreateDesc& rt);
		void DestroyRenderTarget(VulkanRenderTarget* rt);

		VkRenderPass CreateRenderPass(VulkanRenderTarget* pColorRt, VulkanRenderTarget* pDepthRt); // relative to rts
		void DestroyRenderPass(VkRenderPass renderPass);

		VkPipelineCache CreatePipelineCache();
		void DestroyPipelineCache(VkPipelineCache pipelineCache);
		// TODO: remove layout to set descriptions layout (Root Signature)
		VkPipeline CreatePipeline(VkPipelineCache pipelineCache, VkRenderPass renderPass, Shader& outShader);
		void DestroyPipeline(VkPipeline pipeline);

		VulkanQueue* GetQueue(EQueueType type) const;

		// resources (TODO: move to asset manager)
		VulkanBuffer* CreateBuffer(const BufferCreateDesc& bufferCI);
		void          DestroyBuffer(VulkanBuffer* pBuffer);

		bool SupportExtension(const string& extension);
	private:
		int FetchQueueFamilyIndicies(VkQueueFlagBits type);
		UInt32 GetMemoryType(UInt32 typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
	};

}