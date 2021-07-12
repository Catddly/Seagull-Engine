#include "StdAfx.h"
#include "VkRenderer.h"

#include "Common/System/ISystem.h"
#include "Common/System/ILog.h"
#include "Common/Platform/IOperatingSystem.h"
#include "Common/Platform/OsDevices.h"

#include "Common/Stl/vector.h"

namespace SG
{

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
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void _DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	bool VkRenderer::OnInit()
	{
		bool bIsSuccess = true;
		if (!CreateInstance())
		{
			bIsSuccess = false;
			SG_LOG_ERROR("Failed to create vkInstance");
		}

		SetupDebugMessager();
		SelectPhysicalDevice();

		return bIsSuccess;
	}

	void VkRenderer::OnShutdown()
	{
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		_DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessager, nullptr);
#endif
		vkDestroyInstance(mInstance, nullptr);
	}

	bool VkRenderer::CreateInstance()
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

		ValidateExtensions(&insInfo);
		ValidateLayers(&insInfo);

		insInfo.pNext = nullptr;
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		PopulateDebugMsgCreateInfo(debugCreateInfo);
		insInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

		if (vkCreateInstance(&insInfo, nullptr, &mInstance) != VK_SUCCESS)
			return false;

		SG_LOG_DEBUG("Successfully initialized the renderer");
		return true;
	}

	void VkRenderer::ValidateExtensions(VkInstanceCreateInfo* info)
	{
		UInt32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		SG_LOG_DEBUG("Available extension count (%d)", extensionCount);
		for (const auto& extension : extensions) {
			SG_LOG_DEBUG("\t %s", extension.extensionName);
		}
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		mValidateExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		mValidateExtensions.emplace_back("VK_KHR_surface");

		bool bAreAllExtensionValid = true;
		for (const char* name : mValidateExtensions)
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

		info->enabledExtensionCount = (UInt32)mValidateExtensions.size();
		info->ppEnabledExtensionNames = mValidateExtensions.data();
	}

	void VkRenderer::ValidateLayers(VkInstanceCreateInfo* info)
	{
		UInt32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		//SG_LOG_DEBUG("Available layer count (%d)", layerCount);
		//for (const auto& layer : availableLayers) {
		//	SG_LOG_DEBUG("\t %s", layer.layerName);
		//}
#if SG_ENABLE_VK_VALIDATION_LAYER
		mValidateLayers.emplace_back("VK_LAYER_KHRONOS_validation");
		bool bAreAllLayerValid = true;
		for (const char* name : mValidateLayers)
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
			info->enabledLayerCount = (UInt32)mValidateLayers.size();
			info->ppEnabledLayerNames = mValidateLayers.data();
		}
#endif
	}

	void VkRenderer::SetupDebugMessager()
	{
#ifndef SG_ENABLE_VK_VALIDATION_LAYER
		return; // release mode, don't need the debug messager
#endif
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		PopulateDebugMsgCreateInfo(createInfo);

		if (_CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessager) != VK_SUCCESS)
			SG_ASSERT(false);
	}

	void VkRenderer::PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager)
	{
		debugMessager.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessager.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessager.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugMessager.pfnUserCallback = _VkDebugCallback;
		debugMessager.pUserData = nullptr;
	}

	void VkRenderer::SelectPhysicalDevice()
	{
		auto* pOS = System::GetInstance()->GetIOS();
		UInt32 deviceCount = pOS->GetAdapterCount();
		Adapter* adapter = pOS->GetPrimaryAdapter();
		SG_LOG_DEBUG("Adapter Name (%ws)", adapter->GetDisplayName().c_str());

		UInt32 cnt;
		vkEnumeratePhysicalDevices(mInstance, &cnt, nullptr);
		vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());
		for (auto& device : devices) {
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures   deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
			SG_LOG_DEBUG("VkAdapter Name (%s)", deviceProperties.deviceName);
		}
	}

}