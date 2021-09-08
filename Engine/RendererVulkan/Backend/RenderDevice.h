#pragma once

#include "RendererVulkan/Config.h"
#include "Render/Queue.h"
#include "Render/SwapChain.h"
#include "Render/Shader.h"

#include "Render/IRenderDevice.h"
#include "Platform/IOperatingSystem.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"
#include "Stl/string.h"
#include <eastl/optional.h>

namespace SG
{

#ifdef SG_DEBUG
#	define SG_ENABLE_VK_VALIDATION_LAYER 1
#else
#	define SG_ENABLE_VK_VALIDATION_LAYER 0
#endif

	class RenderDeviceVk : public IRenderDevice
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

		struct SwapChain
		{
			VkSwapchainKHR  handle = VK_NULL_HANDLE;
			VkExtent2D      extent;
			vector<Texture>	renderTextures;
		};

		struct Shader
		{
			ShaderStages    stages;
			EShaderLanguage language = EShaderLanguage::eGLSL;
		};

	public:
		SG_RENDERER_VK_API virtual void OnInit() override;
		SG_RENDERER_VK_API virtual void OnShutdown() override;

		virtual void OnUpdate() {}

		SG_RENDERER_VK_API virtual const char* GetRegisterName() const override { return "RenderDevice"; }
	protected:
		bool Initialize();
		void Shutdown();

		bool FetchQueue(Queue& queue, EQueueType type, EQueuePriority priority);
		//void DestroyQueue(Queue& queue);

		bool CreateSwapchain(SwapChain& swapchain, EImageFormat format, EPresentMode presentMode, const Resolution& res);
		void DestroySwapchain(SwapChain& swapchain);

		bool CreateShader(Shader** ppShader, const char** shaderStages, Size numShaderStages);
		void DestroyShader(Shader* pShader);
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

		Shader* mTriangleShader = nullptr;
	};

}