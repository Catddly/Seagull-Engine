#pragma once

#include "RendererVulkan/Config.h"
#include "Common/Core/Defs.h"
#include "Common/Renderer/IRenderer.h"

#include <vulkan/vulkan_core.h>

#include "Common/Stl/vector.h"

namespace SG
{

#ifdef SG_DEBUG
#	define SG_ENABLE_VK_VALIDATION_LAYER 1
#else
#	define SG_ENABLE_VK_VALIDATION_LAYER 0
#endif

	class SG_ALIGN(64) VkRenderer final : public Renderer
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
	private:
		VkInstance mInstance;
		vector<const char*> mValidateExtensions;
		vector<const char*> mValidateLayers;
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		VkDebugUtilsMessengerEXT mDebugMessager;
#endif
		VkPhysicalDevice mPhysicalDevice;        //!< corresponding to the adapter in DeviceManager
	};

}