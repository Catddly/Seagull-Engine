#pragma once

#include "Render/Queue.h"
#include "Render/SwapChain.h"

#include "Platform/IOperatingSystem.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"
#include <eastl/optional.h>

namespace SG
{

#ifdef SG_DEBUG
#	define SG_ENABLE_VK_VALIDATION_LAYER 1
#else
#	define SG_ENABLE_VK_VALIDATION_LAYER 0
#endif

	class VkRenderDevice
	{
	public:
		VkRenderDevice(UInt32 swapchainImageCount = 2);
		~VkRenderDevice();

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

	protected:
		bool Initialize();
		void Shutdown();

		bool CreateQueue(Queue& queue, EQueueType type, EQueuePriority priority);
		//void DestroyQueue(Queue& queue);

		bool CreateSwapchain(SwapChain& swapchain, EImageFormat format, EPresentMode presentMode, const Resolution& res);
		void DestroySwapchain(SwapChain& swapchain);
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

	private:
		UInt32 mSwapChainImageCount;

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

		VkSurfaceKHR     mPresentSurface = VK_NULL_HANDLE;
		Queue            mGraphicQueue; //! Force that the graphic queue and the present queue should be the same.
		SwapChain        mSwapchain;
	};

}