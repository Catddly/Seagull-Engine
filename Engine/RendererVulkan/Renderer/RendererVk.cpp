#include "StdAfx.h"
#include "RendererVk.h"

#include "Common/System/ISystem.h"
#include "Common/System/ILog.h"
#include "Common/Memory/IMemory.h"
#include "Common/Platform/IOperatingSystem.h"
#include "Common/Platform/Window.h"

#include "RendererVulkan/Queue/QueueVk.h"
#include "RendererVulkan/SwapChain/SwapChainVk.h"
#include "RendererVulkan/RenderContext/RenderContextVk.h"

#include "RendererVulkan/Shader/ShaderVk.h"

#include "Common/Stl/vector.h"
#include <EASTL/set.h>

#ifdef SG_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	endif
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
#endif // SG_ENABLE_VK_VALIDATION_LAYER

	bool RendererVk::OnInit()
	{
		auto* pOS = System::GetInstance()->GetIOS();
		Window* window = pOS->GetMainWindow();

		bool bIsSuccess = true;
		if (!CreateInstance())
		{
			bIsSuccess = false;
			SG_LOG_ERROR("Failed to create vkInstance");
		}

		mpRenderContext = new RenderContextVk(this);
		SetupDebugMessager();
		SelectPhysicalDevice();

		mGraphicQueue = new QueueVk(EQueueType::eGraphic, EQueuePriority::eNormal, this);

		CreateLogicalDevice();
		mGraphicQueue->GetNativeHandle(); // Get the queue from the logical device.

		CreateSurface();
		CheckForPresentQueue();
		auto rect = window->GetCurrRect();
		mSwapChain = new SwapChainVk(EImageFormat::eSrgb_B8G8R8A8, EPresentMode::eFIFO, { GetRectWidth(rect), GetRectHeight(rect) }, this);

		mVertShader = new ShaderVk(this, "basic.vert");
		mFragShader = new ShaderVk(this, "basic.frag");

		return bIsSuccess;
	}

	void RendererVk::OnShutdown()
	{
		delete mVertShader;
		delete mFragShader;

		delete mSwapChain;
		if (!mbGraphicQueuePresentable)
			delete mPresentQueue;
		vkDestroySurfaceKHR(mInstance, mpRenderContext->mPresentSurface, nullptr);
		vkDestroyDevice(mpRenderContext->mLogicalDevice, nullptr);
		delete mGraphicQueue;
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		_DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessager, nullptr);
#endif
		vkDestroyInstance(mInstance, nullptr);
	}

	bool RendererVk::CreateInstance()
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

	void RendererVk::ValidateExtensions(VkInstanceCreateInfo* info)
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
#ifdef SG_PLATFORM_WINDOWS
		mValidateExtensions.emplace_back("VK_KHR_win32_surface");
#endif

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

	void RendererVk::ValidateLayers(VkInstanceCreateInfo* info)
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

	void RendererVk::SetupDebugMessager()
	{
#ifndef SG_ENABLE_VK_VALIDATION_LAYER
		return; // release mode, don't need the debug messager
#endif
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		PopulateDebugMsgCreateInfo(createInfo);

		if (_CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessager) != VK_SUCCESS)
			SG_ASSERT(false);
	}

	void RendererVk::PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager)
	{
		debugMessager.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessager.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessager.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugMessager.pfnUserCallback = _VkDebugCallback;
		debugMessager.pUserData = nullptr;
	}

	void RendererVk::SelectPhysicalDevice()
	{
		//auto* pOS = System::GetInstance()->GetIOS();
		//UInt32 deviceCount = pOS->GetAdapterCount();
		//Adapter* adapter = pOS->GetPrimaryAdapter();
		//SG_LOG_DEBUG("Adapter Name (%ws)", adapter->GetDisplayName().c_str());

		UInt32 cnt;
		vkEnumeratePhysicalDevices(mInstance, &cnt, nullptr);
		vector<VkPhysicalDevice> devices(cnt);
		vkEnumeratePhysicalDevices(mInstance, &cnt, devices.data());
		// TODO: add more conditions to choose the best device(adapter)
		for (auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures   deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
			SG_LOG_DEBUG("VkAdapter Name: %s", deviceProperties.deviceName);
	
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)
			{
				mpRenderContext->mPhysicalDevice = device;
				break;
			}
		}
	
	}
	
	void RendererVk::CreateLogicalDevice()
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = mGraphicQueue->GetQueueIndex();
		queueCreateInfo.queueCount = 1;
	
		float queuePriority = 1.0f;
		if (mGraphicQueue->GetPriority() == EQueuePriority::eHigh)
			queuePriority = 2.0f;
		else if (mGraphicQueue->GetPriority() == EQueuePriority::eImmediate)
			queuePriority = 5.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;
	
		VkPhysicalDeviceFeatures deviceFeatures = {};
	
		UInt32 extensionCount;
		vkEnumerateDeviceExtensionProperties(mpRenderContext->mPhysicalDevice, nullptr, &extensionCount, nullptr);
		vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(mpRenderContext->mPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());
	
		// to support swapchain
		const vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		eastl::set<string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
			if (requiredExtensions.empty())
				break;
		}
		if (!requiredExtensions.empty())
			SG_LOG_WARN("Extensions in physical device do not include the ohters in instance");
	
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
	
		createInfo.enabledExtensionCount = (UInt32)deviceExtensions.size();;
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		createInfo.enabledLayerCount = (UInt32)mValidateLayers.size();
		createInfo.ppEnabledLayerNames = mValidateLayers.data();
#endif
	
		if (vkCreateDevice(mpRenderContext->mPhysicalDevice, &createInfo, nullptr, &mpRenderContext->mLogicalDevice) != VK_SUCCESS)
			SG_LOG_ERROR("Failed to create logical device");
	}
	
	void RendererVk::CreateSurface()
	{
		auto* pOS = System::GetInstance()->GetIOS();
		Window* mainWindow = pOS->GetMainWindow();
	
#ifdef SG_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)mainWindow->GetNativeHandle();
		createInfo.hinstance = ::GetModuleHandle(NULL);
	
		if (vkCreateWin32SurfaceKHR(mInstance, &createInfo, nullptr, &mpRenderContext->mPresentSurface) != VK_SUCCESS)
			SG_LOG_ERROR("Failed to create win32 surface");
#endif
	}

	void RendererVk::CheckForPresentQueue()
	{
		VkBool32 presentSupport = false;
		// check if the graphic queue can do the presentation job
		vkGetPhysicalDeviceSurfaceSupportKHR((VkPhysicalDevice)mpRenderContext->GetPhysicalDeviceHandle(), mGraphicQueue->GetQueueIndex(),
			(VkSurfaceKHR)mpRenderContext->GetRenderSurface(), &presentSupport);
		if (!presentSupport)
		{
			SG_LOG_ERROR("Current physical device not support surface presentation");
			mbGraphicQueuePresentable = false;
			mPresentQueue = new QueueVk(EQueueType::eGraphic, EQueuePriority::eNormal, this);
		}
		else
		{
			mbGraphicQueuePresentable = true;
			mPresentQueue = mGraphicQueue;
		}
	}

	Queue* RendererVk::GetGraphicQueue() const
	{
		return mGraphicQueue;
	}

	Queue* RendererVk::GetPresentQueue() const
	{
		return mPresentQueue;
	}

	RenderContext* RendererVk::GetRenderContext() const
	{
		return mpRenderContext;
	}

}