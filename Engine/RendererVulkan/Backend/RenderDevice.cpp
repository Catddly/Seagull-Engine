#include "StdAfx.h"
#include "RenderDevice.h"

#include "Platform/Window.h"
#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include "RendererVulkan/Utils/VulkanConversion.h"

#include <eastl/set.h>

#ifdef SG_PLATFORM_WINDOWS
#	include <windows.h>
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

	void RenderDeviceVk::OnInit()
	{
		Initialize();
	}

	void RenderDeviceVk::OnShutdown()
	{
		Shutdown();
	}

	bool RenderDeviceVk::Initialize()
	{
		if (!CreateVkInstance())
		{
			SG_LOG_ERROR("Failed to create vulkan instance!");
			return false;
		}
		SetupDebugMessenger();

		if (!SelectPhysicalDevice())
		{
			SG_LOG_ERROR("No suittable GPU detected!");
			return false;
		}
		if (!CreateLogicalDevice()) // also create the graphic queue, too.
		{
			SG_LOG_ERROR("Failed to create logical device!");
			return false;
		}

		if (!CreatePresentSurface())
		{
#ifdef SG_PLATFORM_WINDOWS
			SG_LOG_ERROR("Failed to create win32 surface!");
#endif
			return false;
		}

		auto* pOS = CSystem::GetInstance()->GetOS();
		Window* window = pOS->GetMainWindow();
		auto rect = window->GetCurrRect();
		Resolution swapchainRes = { GetRectWidth(rect), GetRectHeight(rect) };
		if (!CreateSwapchain(mSwapchain, EImageFormat::eSrgb_B8G8R8A8, EPresentMode::eFIFO, swapchainRes))
		{
			SG_LOG_ERROR("Failed to create swapchain!");
			return false;
		}

		const char* stages[] = { "basic.vert", "basic.frag" };
		CreateShader(&mTriangleShader, stages, 2);

		DestroyShader(mTriangleShader);

		return true;
	}

	void RenderDeviceVk::Shutdown()
	{
		DestroySwapchain(mSwapchain);
		vkDestroySurfaceKHR(mInstance.instance, mInstance.presentSurface, nullptr);
		vkDestroyDevice(mInstance.logicalDevice, nullptr);
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		_DestroyDebugUtilsMessengerEXT(mInstance.instance, mInstance.debugLayer, nullptr);
#endif
		vkDestroyInstance(mInstance.instance, nullptr);
	}

	bool RenderDeviceVk::FetchQueue(Queue& queue, EQueueType type, EQueuePriority priority)
	{
		queue.priority = priority;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mInstance.physicalDevice, &queueFamilyCount, nullptr);
		vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(mInstance.physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			bool bFoundQueue = false;
			switch (type)
			{
			case EQueueType::eNull:
				SG_LOG_ERROR("Invalid Queue Type!");
				break;
			case EQueueType::eGraphic:
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					queue.queueFamilyIndex = i;
				bFoundQueue = true;
				break;
			case EQueueType::eCompute:
				if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
					queue.queueFamilyIndex = i;
				bFoundQueue = true;
				break;
			case EQueueType::eTransfer:
				if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
					queue.queueFamilyIndex = i;
				bFoundQueue = true;
				break;
			}
			if (bFoundQueue)
				break;
			i++;
		}

		if (mInstance.logicalDevice != VK_NULL_HANDLE)
			vkGetDeviceQueue(mInstance.logicalDevice, queue.queueFamilyIndex.value(), 0, &queue.handle);

		return true;
	}

	bool RenderDeviceVk::CreateSwapchain(SwapChain& swapchain, EImageFormat format, EPresentMode presentMode, const Resolution& res)
	{
		VkSurfaceCapabilitiesKHR capabilities = {};
		vector<VkSurfaceFormatKHR> formats;
		vector<VkPresentModeKHR> presentModes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mInstance.physicalDevice, mInstance.presentSurface, &capabilities);

		UInt32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mInstance.physicalDevice, mInstance.presentSurface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mInstance.physicalDevice, mInstance.presentSurface, &formatCount, formats.data());
		}

		UInt32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mInstance.physicalDevice, mInstance.presentSurface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mInstance.physicalDevice, mInstance.presentSurface, &presentModeCount, presentModes.data());
		}

		// if the swapchain can do presenting
		bool bIsSwapChainAdequate = false;
		bIsSwapChainAdequate = !formats.empty() && !presentModes.empty();
		if (!bIsSwapChainAdequate)
			SG_LOG_WARN("Unpresentable swapchain detected");

		bool bFormatSupported = false;
		VkColorSpaceKHR colorSpace = {};
		for (auto& f : formats)
		{
			if (f.format == ToVkImageFormat(format))
			{
				bFormatSupported = true;
				colorSpace = f.colorSpace;
				break;
			}
		}
		if (!bFormatSupported)
			SG_LOG_WARN("Unsupported image format");

		bool bPresentModeSupported = false;
		for (auto& pm : presentModes)
		{
			if (pm == ToVkPresentMode(presentMode))
			{
				bPresentModeSupported = true;
				break;
			}
		}
		if (!bPresentModeSupported)
			SG_LOG_WARN("Unsupported present format");

		// check for the swapchain extent
		VkExtent2D extent = { res.width, res.height };
		extent.width  = eastl::clamp(res.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = eastl::clamp(res.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		swapchain.extent.width = extent.width;
		swapchain.extent.height = extent.height;

		//UInt32 imageCount = capabilities.minImageCount + 1;
		//if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		//	imageCount = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mInstance.presentSurface;
		//createInfo.minImageCount = imageCount;
		createInfo.minImageCount = SG_SWAPCHAIN_IMAGE_COUNT;
		createInfo.imageFormat = ToVkImageFormat(format);
		createInfo.imageColorSpace = colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// force to make the graphic queue as the present queue
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = ToVkPresentMode(presentMode);
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(mInstance.logicalDevice, &createInfo, nullptr, &swapchain.handle) != VK_SUCCESS)
			return false;

		swapchain.renderTextures.resize(SG_SWAPCHAIN_IMAGE_COUNT);
		// fetch image from swapchain
		UInt32 imageCnt;
		vkGetSwapchainImagesKHR(mInstance.logicalDevice, swapchain.handle, &imageCnt, nullptr);
		vector<VkImage> vkImages;  
		vkImages.resize(imageCnt);
		vkGetSwapchainImagesKHR(mInstance.logicalDevice, swapchain.handle, &imageCnt, vkImages.data());

		for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
		{
			swapchain.renderTextures[i].resolution = { swapchain.extent.width, swapchain.extent.height };
			swapchain.renderTextures[i].format = format;
			swapchain.renderTextures[i].arrayCount = 1;
			swapchain.renderTextures[i].image = vkImages[i];

			// create image view for swapchain image
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapchain.renderTextures[i].image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
			// no swizzle
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(mInstance.logicalDevice, &createInfo, nullptr, &swapchain.renderTextures[i].imageView) != VK_SUCCESS)
			{
				SG_LOG_ERROR("Failed to create swapchain image view!");
				return false;
			}
		}

		return true;
	}

	void RenderDeviceVk::DestroySwapchain(SwapChain& swapchain)
	{
		for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
			vkDestroyImageView(mInstance.logicalDevice, swapchain.renderTextures[i].imageView, nullptr);
		vkDestroySwapchainKHR(mInstance.logicalDevice, swapchain.handle, nullptr);
	}

	bool RenderDeviceVk::CreateShader(Shader** ppShader, const char** shaderStages, Size numShaderStages)
	{
		*ppShader = Memory::New<Shader>();
		auto* pShader = *ppShader;
		for (Size i = 0; i < numShaderStages; i++)
		{
			string stage = string(shaderStages[i]);

			Size dotPos = stage.find_last_of('.');
			string name = stage.substr(0, dotPos);
			string extension = stage.substr(dotPos + 1, stage.size() - dotPos);

			auto* pFS = CSystem::GetInstance()->GetFileSystem();
			if (extension == "spv")
			{
				if (!ReadBinaryFromDisk(pShader, name, extension)) // no spirv file exist, try to compile the file from ShaderSrc
				{
					Size slashPos = stage.find_last_of('-');
					if (slashPos == string::npos)
					{
						SG_LOG_ERROR("Invalid spv file format. (spv format: shader-vert.spv)");
						SG_ASSERT(false);
					}
					string compiledPath = stage.substr(0, dotPos);  // convert to string to ensure the null-terminated string
					compiledPath[slashPos] = '.';
					string name = compiledPath.substr(0, slashPos);
					string extension = compiledPath.substr(slashPos + 1, compiledPath.size() - slashPos);
					CompileUseVulkanSDK(name, extension);
					if (!ReadBinaryFromDisk(pShader, name, extension))
					{
						SG_LOG_ERROR("No such file exists! (%s)", stage.c_str());
						// TODO: no assert, but throw an error.
						SG_ASSERT(false);
					}
				}
			}
			else if (extension == "vert" || extension == "frag" || extension == "comp" || extension == "geom" || extension == "tesc" || extension == "tese") // compiled the source shader to spirv
			{
				if (!ReadBinaryFromDisk(pShader, name, extension)) // no spirv file exist, try to compile the file from ShaderSrc
				{
					CompileUseVulkanSDK(name, extension);
					if (!ReadBinaryFromDisk(pShader, name, extension))
					{
						SG_LOG_ERROR("No such file exists! (%s.%s)", name, extension);
						SG_ASSERT(false);
					}
				}
			}
			else
			{
				SG_LOG_WARN("Unknown file type of shader!");
				SG_ASSERT(false);
			}

			if (extension == "spv")
			{
				Size slashPos = name.find_last_of('-');
				extension = name.substr(slashPos + 1, name.size() - slashPos);
			}
			if (extension == "vert")
				CreateVulkanShaderModule(pShader->stages[EShaderStages::efVert]);
			else if (extension == "frag")
				CreateVulkanShaderModule(pShader->stages[EShaderStages::efFrag]);
			else if (extension == "comp")
				CreateVulkanShaderModule(pShader->stages[EShaderStages::efComp]);
			else if (extension == "gemo")
				CreateVulkanShaderModule(pShader->stages[EShaderStages::efGeom]);
			else if (extension == "tesc")
				CreateVulkanShaderModule(pShader->stages[EShaderStages::efTesc]);
			else if (extension == "tese")
				CreateVulkanShaderModule(pShader->stages[EShaderStages::efTese]);
		}
		return true;
	}
	
	void RenderDeviceVk::DestroyShader(Shader* pShader)
	{
		for (auto beg = pShader->stages.begin(); beg != pShader->stages.end(); beg++)
		{
			Memory::Free(beg->second.pBinary);
			VkShaderModule shaderModule = (VkShaderModule)beg->second.pShader;
			vkDestroyShaderModule(mInstance.logicalDevice, shaderModule, nullptr);
		}
		Memory::Delete(pShader);
		pShader = nullptr;
	}

	bool RenderDeviceVk::CreateVkInstance()
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

		// fill in the neccessary validation data
		ValidateExtensions(&insInfo);
		ValidateLayers(&insInfo);

		insInfo.pNext = nullptr;

#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		PopulateDebugMsgCreateInfo(debugCreateInfo);
		insInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

		if (vkCreateInstance(&insInfo, nullptr, &mInstance.instance) != VK_SUCCESS)
			return false;
		return true;
	}

	void RenderDeviceVk::ValidateExtensions(VkInstanceCreateInfo* info)
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
		mInstance.validateExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		mInstance.validateExtensions.emplace_back("VK_KHR_surface");
#ifdef SG_PLATFORM_WINDOWS
		mInstance.validateExtensions.emplace_back("VK_KHR_win32_surface");
#endif

		bool bAreAllExtensionValid = true;
		for (const char* name : mInstance.validateExtensions)
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

		info->enabledExtensionCount   = (UInt32)mInstance.validateExtensions.size();
		info->ppEnabledExtensionNames = mInstance.validateExtensions.data();
	}

	void RenderDeviceVk::ValidateLayers(VkInstanceCreateInfo* info)
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
		mInstance.validateLayers.emplace_back("VK_LAYER_KHRONOS_validation");
		bool bAreAllLayerValid = true;
		for (const char* name : mInstance.validateLayers)
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
			info->enabledLayerCount = (UInt32)mInstance.validateLayers.size();
			info->ppEnabledLayerNames = mInstance.validateLayers.data();
		}
#endif
	}

	void RenderDeviceVk::PopulateDebugMsgCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessager)
	{
		debugMessager.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessager.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessager.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugMessager.pfnUserCallback = _VkDebugCallback;
		debugMessager.pUserData = nullptr;
	}

	void RenderDeviceVk::SetupDebugMessenger()
	{
#ifndef SG_ENABLE_VK_VALIDATION_LAYER
		return; // release mode, don't need the debug messenger
#endif
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		PopulateDebugMsgCreateInfo(createInfo);

		if (_CreateDebugUtilsMessengerEXT(mInstance.instance, &createInfo, nullptr, &mInstance.debugLayer) != VK_SUCCESS)
			SG_ASSERT(false);
	}

	bool RenderDeviceVk::SelectPhysicalDevice()
	{
		//auto* pOS = System::GetInstance()->GetIOS();
		//UInt32 deviceCount = pOS->GetAdapterCount();
		//Adapter* adapter = pOS->GetPrimaryAdapter();
		//SG_LOG_DEBUG("Adapter Name (%ws)", adapter->GetDisplayName().c_str());

		UInt32 cnt;
		vkEnumeratePhysicalDevices(mInstance.instance, &cnt, nullptr);
		vector<VkPhysicalDevice> devices(cnt);
		vkEnumeratePhysicalDevices(mInstance.instance, &cnt, devices.data());

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
				mInstance.physicalDevice = device;
				return true;
			}

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			{
				mInstance.physicalDevice = device;
				SG_LOG_WARN("Integrated GPU detected!");
				return true;
			}
		}

		return false;
	}

	bool RenderDeviceVk::CreateLogicalDevice()
	{
		FetchQueue(mGraphicQueue, EQueueType::eGraphic, EQueuePriority::eNormal);

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = mGraphicQueue.queueFamilyIndex.value();
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		if (mGraphicQueue.priority == EQueuePriority::eHigh)
			queuePriority = 2.0f;
		else if (mGraphicQueue.priority == EQueuePriority::eImmediate)
			queuePriority = 5.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		UInt32 extensionCount;
		vkEnumerateDeviceExtensionProperties(mInstance.physicalDevice, nullptr, &extensionCount, nullptr);
		vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(mInstance.physicalDevice, nullptr, &extensionCount, availableExtensions.data());

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
			SG_LOG_WARN("Extensions in physical device do not include the others in instance");

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount   = (UInt32)deviceExtensions.size();;
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		createInfo.enabledLayerCount   = (UInt32)mInstance.validateLayers.size();
		createInfo.ppEnabledLayerNames = mInstance.validateLayers.data();
#endif

		if (vkCreateDevice(mInstance.physicalDevice, &createInfo, nullptr, &mInstance.logicalDevice) != VK_SUCCESS)
			return false;

		// fetch current basis graphic queue
		vkGetDeviceQueue(mInstance.logicalDevice, mGraphicQueue.queueFamilyIndex.value(), 0, &mGraphicQueue.handle);
		return true;
	}

	bool RenderDeviceVk::CreatePresentSurface()
	{
		auto* pOS = CSystem::GetInstance()->GetOS();
		Window* mainWindow = pOS->GetMainWindow();

#ifdef SG_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)mainWindow->GetNativeHandle();
		createInfo.hinstance = ::GetModuleHandle(NULL);

		if (vkCreateWin32SurfaceKHR(mInstance.instance, &createInfo, nullptr, &mInstance.presentSurface) != VK_SUCCESS)
		{
			return false;
		}
		else
		{
			// check if the graphic queue can do the presentation job
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(mInstance.physicalDevice, mGraphicQueue.queueFamilyIndex.value(), mInstance.presentSurface, &presentSupport);
			if (!presentSupport)
			{
				SG_LOG_ERROR("Current physical device not support surface presentation");
				return false;
			}
			return true;
		}
#endif
	}

	string RenderDeviceVk::CompileUseVulkanSDK(const string& name, const string& extension) const
	{
		char* glslc = "";
		Size num = 1;
		_dupenv_s(&glslc, &num, "VULKAN_SDK");
		glslc[num + 1] = '\0';
		strcat_s(glslc, sizeof(wchar_t) * (num + 1), "\\Bin32\\glslc.exe");
		string compiledName = name + "-" + extension + ".spv";
		string shaderPath = CSystem::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Sources) + name + "." + extension;
		string outputPath = CSystem::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Binarires) + compiledName;

		const char* args[3] = { shaderPath.c_str(), "-o", outputPath.c_str() };
		string pOut = CSystem::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Binarires) + name + "-" + extension + "-compile.log";

		// create a process to use vulkanSDK to compile shader sources to binary (spirv)
		if (CSystem::GetInstance()->RunProcess(glslc, args, 3, pOut.c_str()) != 0)
		{
			SG_LOG_WARN("%s", pOut);
			SG_ASSERT(false);
		}

		return eastl::move(compiledName);
	}

	bool RenderDeviceVk::ReadBinaryFromDisk(Shader* pShader, const string& name, const string& extension)
	{
		auto* pFS = CSystem::GetInstance()->GetFileSystem();
		string filepath = "";
		if (extension == "spv")
			filepath = name + "." + extension;
		else
			filepath = name + "-" + extension + ".spv";
		if (pFS->Open(EResourceDirectory::eShader_Binarires, filepath.c_str(), EFileMode::efRead_Binary))
		{
			Size binarySize = pFS->FileSize();
			std::byte* pBinary = (std::byte*)Memory::Malloc(binarySize);
			pFS->Read(pBinary, binarySize);

			string actualExtension = extension;
			if (extension == "spv")
			{
				Size slashPos = name.find_last_of('-');
				actualExtension = name.substr(slashPos + 1, name.size() - slashPos);
			}

			if (actualExtension == "vert")
				pShader->stages[EShaderStages::efVert] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "frag")
				pShader->stages[EShaderStages::efFrag] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "comp")
				pShader->stages[EShaderStages::efComp] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "gemo")
				pShader->stages[EShaderStages::efGeom] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "tese")
				pShader->stages[EShaderStages::efTese] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "tesc")
				pShader->stages[EShaderStages::efTesc] = { pBinary, binarySize, VK_NULL_HANDLE };

			pFS->Close();
			return true;
		}
		else
			return false;
	}

	void RenderDeviceVk::CreateVulkanShaderModule(ShaderData& pShaderData)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = pShaderData.binarySize;
		createInfo.pCode = reinterpret_cast<UInt32*>(pShaderData.pBinary);

		VkShaderModule shaderModule = {};
		if (vkCreateShaderModule(mInstance.logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create shader module from spirv!");
			SG_ASSERT(false);
		}
		pShaderData.pShader = shaderModule;
	}

}