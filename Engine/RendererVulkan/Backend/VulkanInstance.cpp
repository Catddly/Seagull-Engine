#include "StdAfx.h"
#include "VulkanInstance.h"

#include "System/ILogger.h"
#include "Platform/IOperatingSystem.h"
#include "Platform/Window.h"
#include "Memory/IMemory.h"

#include "Render/Shader.h"

#include "Stl/vector.h"

#ifdef SG_PLATFORM_WINDOWS
#	include <vulkan/vulkan_win32.h>
#endif

namespace SG
{

#ifdef SG_ENABLE_VK_VALIDATION_LAYER

	static VKAPI_ATTR VkBool32 VKAPI_CALL _VkDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			SG_LOG_WARN("Renderer[%s] code(%i): %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			SG_LOG_ERROR("Renderer[%s] code(%i): %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
			SG_ASSERT(false); // TODO: replace to exception.
		}
		return VK_FALSE;
	}

	static VkResult _CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void     _DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

#endif // SG_ENABLE_VK_VALIDATION_LAYER

	VulkanInstance::VulkanInstance()
	{
		if (!CreateVkInstance())
		{
			SG_LOG_ERROR("Failed to create vulkan instance!");
			SG_ASSERT(false);
		}
	}

	VulkanInstance::~VulkanInstance()
	{
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		_DestroyDebugUtilsMessengerEXT(mInstance, mDebugLayer, nullptr);
#endif
		vkDestroyInstance(mInstance, nullptr);
	}

	bool VulkanInstance::CreateVkInstance()
	{
		VkApplicationInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.pApplicationName = SG_ENGINE_NAME;
		info.applicationVersion = VK_MAKE_VERSION(SG_ENGINE_VERSION_MAJOR, SG_ENGINE_VERSION_MINOR, SG_ENGINE_VERSION_PATCH);
		info.pEngineName = SG_ENGINE_NAME;
		info.engineVersion = VK_MAKE_VERSION(SG_ENGINE_VERSION_MAJOR, SG_ENGINE_VERSION_MINOR, SG_ENGINE_VERSION_PATCH);

		info.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo insInfo = {};
		insInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		insInfo.pApplicationInfo = &info;

		insInfo.enabledExtensionCount = 0;
		insInfo.ppEnabledExtensionNames = nullptr;
		insInfo.enabledLayerCount = 0;
		insInfo.ppEnabledLayerNames = nullptr;

		// fill in the necessary validation data
		vector<const char*> extents;
		vector<const char*> layers;

		ValidateExtensions(extents, &insInfo);

		insInfo.pNext = nullptr;

#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		ValidateLayers(layers, &insInfo);

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		PopulateDebugMsgCreateInfo(debugCreateInfo);
		insInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

		if (vkCreateInstance(&insInfo, nullptr, &mInstance) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create vulkan instance");
			return false;
		}

#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		if (_CreateDebugUtilsMessengerEXT(mInstance, &debugCreateInfo, nullptr, &mDebugLayer) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create debug layer");
			SG_ASSERT(false);
		}
#endif

		return true;
	}

	void VulkanInstance::ValidateExtensions(vector<const char*>& extents, VkInstanceCreateInfo* info)
	{
		UInt32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		//SG_LOG_DEBUG("Available extension count (%d)", extensionCount);
		//for (const auto& extension : extensions) 
		// {
		//	SG_LOG_DEBUG("\t %s", extension.extensionName);
		// }

#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		extents.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		extents.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef SG_PLATFORM_WINDOWS
		extents.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		bool bAreAllExtensionValid = true;
		for (const char* name : extents)
		{
			bool bIsFound = false;
			for (auto& ext : extensions)
			{
				if (strcmp(ext.extensionName, name))
				{
					bIsFound = true;
					break;
				}
			}

			if (!bIsFound)
			{
				SG_LOG_ERROR("Vk ext missing (%s)", name);
				bAreAllExtensionValid = false;
			}
		}

		info->enabledExtensionCount   = (UInt32)extents.size();
		info->ppEnabledExtensionNames = extents.data();
	}

	void VulkanInstance::ValidateLayers(vector<const char*>& layers, VkInstanceCreateInfo* info)
	{
		UInt32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//SG_LOG_DEBUG("Available layer count (%d)", layerCount);
		//for (const auto& layer : availableLayers) 
		// {
		//	SG_LOG_DEBUG("\t %s", layer.layerName);
		// }

#if SG_ENABLE_VK_VALIDATION_LAYER
		layers.emplace_back("VK_LAYER_KHRONOS_validation");
		bool bAreAllLayerValid = true;
		for (const char* name : layers)
		{
			bool bIsFound = false;
			for (auto& layer : availableLayers)
			{
				if (strcmp(layer.layerName, name))
				{
					bIsFound = true;
					break;
				}
			}

			if (!bIsFound)
			{
				SG_LOG_ERROR("Vk layer missing (%s)", name);
				bAreAllLayerValid = false;
			}
		}

		if (bAreAllLayerValid)
		{
			info->enabledLayerCount = (UInt32)layers.size();
			info->ppEnabledLayerNames = layers.data();
		}
#endif
	}

	void VulkanInstance::PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager)
	{
		debugMessager.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessager.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessager.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugMessager.pfnUserCallback = _VkDebugCallback;
		debugMessager.pUserData = nullptr;
	}

	void VulkanInstance::SetupDebugMessenger()
	{
#ifndef SG_ENABLE_VK_VALIDATION_LAYER
		return; // release mode, don't need the debug messenger
#endif
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		PopulateDebugMsgCreateInfo(createInfo);

		if (_CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugLayer) != VK_SUCCESS)
			SG_ASSERT(false);
	}

	bool VulkanInstance::SelectPhysicalDeviceAndCreateDevice()
	{
		UInt32 gpuCount;
		vkEnumeratePhysicalDevices(mInstance, &gpuCount, nullptr);
		vector<VkPhysicalDevice> devices(gpuCount);
		vkEnumeratePhysicalDevices(mInstance, &gpuCount, devices.data());

		// TODO: add more conditions to choose the best device(adapter)
		for (auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures   deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
			//SG_LOG_DEBUG("VkAdapter Name: %s", deviceProperties.deviceName);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				mDevice = Memory::New<VulkanDevice>(device);
				return true;
			}

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			{
				mDevice = Memory::New<VulkanDevice>(device);
				SG_LOG_WARN("Integrated GPU detected!");
				return true;
			}
		}

		return false;
	}

}