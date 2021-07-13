#pragma once

#include "RendererVulkan/Config.h"
#include "Common/Core/Defs.h"
#include "Common/Render/Renderer.h"

#include <vulkan/vulkan_core.h>

#include "Common/Stl/vector.h"

namespace SG
{

#ifdef SG_DEBUG
#	define SG_ENABLE_VK_VALIDATION_LAYER 1
#else
#	define SG_ENABLE_VK_VALIDATION_LAYER 0
#endif

	class QueueVk;
	class SwapChainVk;
	class SG_ALIGN(64) RendererVk final : public Renderer
	{
	public:
		SG_RENDERER_VK_API virtual bool OnInit() override;
		SG_RENDERER_VK_API virtual void OnShutdown() override;
	private:
		bool CreateInstance();
		void ValidateExtensions(VkInstanceCreateInfo* info);
		void ValidateLayers(VkInstanceCreateInfo* info);
		void SetupDebugMessager();
		void PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager);
		//! Fetch the info from the DeviceManager
		void SelectPhysicalDevice(); // TODO: support multi-GPU
		void CreateLogicalDevice();
		void CreateSurface();
	private:
		friend class QueueVk;
		friend class SwapChainVk;
	private:
		VkInstance mInstance = VK_NULL_HANDLE;
		vector<const char*> mValidateExtensions;
		vector<const char*> mValidateLayers;
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		VkDebugUtilsMessengerEXT mDebugMessager = VK_NULL_HANDLE;
#endif
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;        //!< corresponding to the adapter in DeviceManager
		VkDevice         mLogicalDevice  = VK_NULL_HANDLE;
		VkSurfaceKHR     mPresentSurface = VK_NULL_HANDLE;

		QueueVk* mGraphicQueue = nullptr;
		QueueVk* mPresentQueue = nullptr;
		bool     mbGraphicQueuePresentable = false;

		SwapChainVk* mSwapChain = nullptr;
	};

}