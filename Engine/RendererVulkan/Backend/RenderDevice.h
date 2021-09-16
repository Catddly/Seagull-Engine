#pragma once

#include "Defs/Defs.h"

#include "RendererVulkan/Config.h"
#include "Render/Queue.h"
#include "Render/SwapChain.h"
#include "Render/Shader.h"
#include "Render/Pipeline.h"
#include "Render/ResourceBarriers.h"

#include "Render/IRenderDevice.h"
#include "Platform/IOperatingSystem.h"
#include "System/ISystemMessage.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"
#include "Stl/string.h"
#include <eastl/optional.h>
#include <eastl/variant.h>

namespace SG
{

#ifdef SG_DEBUG
#	define SG_ENABLE_VK_VALIDATION_LAYER 1
#else
#	define SG_ENABLE_VK_VALIDATION_LAYER 0
#endif

	class RenderDeviceVk : public IRenderDevice, public ISystemMessageListener
	{
	public:
		RenderDeviceVk() = default;
		~RenderDeviceVk() = default;

		struct Queue
		{
			eastl::optional<UInt32>  queueFamilyIndex;
			EQueueType     type = EQueueType::eNull;
			EQueuePriority priority = EQueuePriority::eNormal;
			VkQueue handle = VK_NULL_HANDLE;
		};

		struct SG_ALIGN(64) Texture
		{
			VkImage      image      = VK_NULL_HANDLE;
			VkImageView  imageView  = VK_NULL_HANDLE;
			EImageFormat format     = EImageFormat::eNull;
			Resolution   resolution = { 0, 0 };
			UInt32       arrayCount = 1;
		};
		//SG_COMPILE_ASSERT(sizeof(Texture) == sizeof(UInt64), "Texture can not align to cache line size!");

		struct RenderTarget
		{
			Texture* pTexture = nullptr;
			bool     bUsed    = false;
		};

		struct SwapChain
		{
			VkSwapchainKHR       handle = VK_NULL_HANDLE;
			VkExtent2D           extent;
			EImageFormat         format = EImageFormat::eNull;
			vector<RenderTarget> renderTargets;
		};

		struct Shader
		{
			ShaderStages    stages;
			EShaderLanguage language = EShaderLanguage::eGLSL;
		};

		struct RenderPass
		{
			VkRenderPass handle = VK_NULL_HANDLE;
		};

		struct Pipeline
		{
			VkPipelineLayout layout = VK_NULL_HANDLE;
			VkPipeline       handle = VK_NULL_HANDLE;
			EPipelineType    type   = EPipelineType::eGraphic;
		};

		struct FrameBuffer
		{
			vector<VkFramebuffer> handles;
		};

		//! Where the commands are generated from.
		//! Corresponding to the Queue family to submit the command.
		struct CommandPool
		{
			VkCommandPool handle = VK_NULL_HANDLE;
		};

		struct RenderCommand
		{
			VkCommandBuffer pCommandBuffer;
			RenderPass*     pCurrentRenderPass = nullptr;
			bool            bActiveRenderPass = false;
		};

		struct ResourceBarrier
		{
			// TODO: fill it with other resource type.
			eastl::variant<RenderTarget*> pResource = {};
			EResoureceBarrier oldState;
			EResoureceBarrier newState;
		};

		struct Semaphore
		{
			VkSemaphore handle = VK_NULL_HANDLE;
		};

		struct Fence
		{
			VkFence handle = VK_NULL_HANDLE;
		};

	public:
		SG_RENDERER_VK_API virtual void OnInit() override;
		SG_RENDERER_VK_API virtual void OnShutdown() override;

		SG_RENDERER_VK_API virtual void OnUpdate() override;

		SG_RENDERER_VK_API virtual const char* GetRegisterName() const override { return "RenderDevice"; }

		SG_RENDERER_VK_API virtual bool OnSystemMessage(ESystemMessage msg) override;
	protected:
		bool Initialize();
		void Shutdown();

		bool FetchQueue(Queue& queue, EQueueType type, EQueuePriority priority);
		//void DestroyQueue(Queue& queue);

		bool CreateSwapchain(SwapChain& swapchain, EImageFormat format, EPresentMode presentMode, const Resolution& res);
		void DestroySwapchain(SwapChain& swapchain);

		bool CreateShader(Shader** ppShader, const char** shaderStages, Size numShaderStages);
		void DestroyShader(Shader* pShader);

		bool CreatePipeline(Pipeline** ppPipeline, EPipelineType type, const Shader* const pShader);
		void DestroyPipeline(Pipeline* pPipeline);

		bool CreateFrameBuffer(FrameBuffer** ppFrameBuffer, Pipeline* pPipeline);
		void DestroyFrameBuffer(FrameBuffer* pFrameBuffer);

		bool CreateSemaphores(Semaphore** ppSemaphore);
		void DestroySemaphores(Semaphore* pSemaphore);

		bool CreateFence(Fence** ppFence);
		void DestroyFence(Fence* pFence);
	private:
		/// begin create VkInstance and debug layer
		bool CreateVkInstance();
		void ValidateExtensions(VkInstanceCreateInfo* info);
		void ValidateLayers(VkInstanceCreateInfo* info);
		void PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager);
		void SetupDebugMessenger();
		/// end create VkInstance and debug layer

		/// begin devices and queue creation
		bool SelectPhysicalDevice(); // TODO: support multi-GPU
		bool CreateLogicalDevice();
		bool CreatePresentSurface();
		/// end devices and queue creation

		/// begin shader complilation (Should move to a specific shader compiler)
		//! Compile shader by using glslc.exe from vulkanSDK.
		//! @param (name) the name of the shader.
		//! @param (extension) the extension of the shader.
		//! @return the full name of compiled shader (spirv).
		string CompileUseVulkanSDK(const string& name, const string& extension) const;
		//! Read the compiled shader (spirv) from disk.
		//! @param (filepath) where to get the binary.
		//! @return true if the binary is exist otherwise false.
		bool ReadBinaryFromDisk(Shader* ppShader, const string& name, const string& extension);
		void CreateVulkanShaderModule(ShaderData& pShaderData);
		/// end shader complilation

		/// begin pipeline creation
		bool CreateRenderPass(RenderPass** ppRenderPass);
		void DestroyRenderPass(RenderPass* pRenderPass);
		/// end pipeline creation

		/// begin command relative
		bool CreateCommandPool(CommandPool** ppCommandPool, Queue* pQueue);
		void DestroyCommandPool(CommandPool* pCommandPool);
		bool AllocateCommandBuffer(RenderCommand** ppBuffers, CommandPool* pPool);

		void BeginCommand(RenderCommand* pBuffers);
		void CmdBindPipeline(RenderCommand* pBuffers, Pipeline* pPipeline);
		void CmdBindRenderTarget(RenderCommand* pBuffers, RenderTarget** ppRenderTargets, UInt32 numRts, UInt32 frameIndex);
		void CmdDraw(RenderCommand* pBuffers, UInt32 vertexCnt, UInt32 instanceCnt, UInt32 firstVertex, UInt32 firstInstance);
		void CmdRenderTargetResourceBarrier(RenderCommand* pBuffers, UInt32 numRTs, ResourceBarrier* ppResourceBarries);
		void CmdSetViewport(RenderCommand* pBuffers, float xOffset, float yOffset, float width, float height, float minDepth, float maxDepth);
		void CmdSetScissor(RenderCommand* pBuffers, UInt32 xOffset, UInt32 yOffset, UInt32 width, UInt32 height);
		void EndCommand(RenderCommand* pBuffers);

		void DestroyCommandBuffer(RenderCommand* pBuffers);
		/// end command relative

		/// begin presentation
		void AcquireNextImage(Semaphore* pWaitSemaphore, UInt32& imageIndex, UInt64 timeOut = UINT64_MAX);
		void QueueSubmit(Semaphore* pWaitSemaphore, Semaphore* pSignalSemaphore, Fence* pCPU2GPUCommandExecutedFence, RenderCommand* pCommand);
		void QueuePresent(Semaphore* pWaitSemaphore, UInt32 frameIndex);
		void WaitQueueIdle(Queue* pQueue);
		void WaitForFence(Fence* pFence, UInt64 timeOut = UINT64_MAX);
		/// end presentation

	private:
		//! All these data are singleton in one render device.
		struct SingletonData
		{
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
			VkDebugUtilsMessengerEXT debugLayer = VK_NULL_HANDLE;
#endif
			VkInstance          instance       = VK_NULL_HANDLE;
			VkPhysicalDevice    physicalDevice = VK_NULL_HANDLE; //!< corresponding to the adapter in DeviceManager
			VkDevice            logicalDevice  = VK_NULL_HANDLE;
			VkSurfaceKHR        presentSurface = VK_NULL_HANDLE;
			vector<const char*> validateExtensions;
			vector<const char*> validateLayers;
		} mInstance;

		VkSurfaceKHR  mPresentSurface = VK_NULL_HANDLE;
		Queue         mGraphicQueue; //! Force that the graphic queue and the present queue should be the same.
		SwapChain     mSwapchain;

		Shader*      mpTriangleShader = nullptr;
		Pipeline*    mpDefaultPipeline = nullptr;
		FrameBuffer* mpFrameBuffer = nullptr;
		RenderPass*  mpRenderPass = nullptr;

		CommandPool*   mpCommandPool = nullptr;
		RenderCommand* mpRenderCommmands[SG_SWAPCHAIN_IMAGE_COUNT] = {};

		// GPU to GPU synchronization
		Semaphore* mpFetchImageSemaphore[SG_SWAPCHAIN_IMAGE_COUNT] = {};
		Semaphore* mpRenderFinishSemaphore[SG_SWAPCHAIN_IMAGE_COUNT] = {};
		// CPU to GPU synchronization
		Fence*     mpInFlightFence[SG_SWAPCHAIN_IMAGE_COUNT] = {};
		Fence*     mpImageInFlightFence[SG_SWAPCHAIN_IMAGE_COUNT] = {};
	};

}