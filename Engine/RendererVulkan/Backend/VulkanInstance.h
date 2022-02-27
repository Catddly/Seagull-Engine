#pragma once

#include "RendererVulkan/Config.h"
#include "Render/SwapChain.h"

#include "VulkanConfig.h"
#include "volk.h"

#include "Stl/vector.h"

namespace SG
{

	//! @brief All the singleton data in one render device.
	//! Should be initialized just once and its life cycle is the same
	//! as the render device. 
	class VulkanInstance
	{
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanInstance);
	public:
		VulkanInstance();
		~VulkanInstance();

		VkInstance       instance = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice;
	private:
		bool CreateVkInstance();
		void ValidateExtensions(vector<const char*>& extens, VkInstanceCreateInfo* info);
		void ValidateLayers(vector<const char*>& layers, VkInstanceCreateInfo* info);
		void PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager);
		void SetupDebugMessenger();
		bool SelectPhysicalDevice();
	private:
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		VkDebugUtilsMessengerEXT mDebugLayer;
#endif
	};

}