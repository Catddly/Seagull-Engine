#include "StdAfx.h"
#include "RenderDevice.h"

#include "Platform/Window.h"
#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include "RendererVulkan/Utils/VkConvert.h"

#include "RendererVulkan/Backend/VulkanInstance.h"

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

	void RenderDeviceVk::OnInit()
	{
		//SSystem()->RegisterSystemMessageListener(this);
		mVulkanInstance = Memory::New<VulkanInstance>();
		//Initialize();
	}

	void RenderDeviceVk::OnShutdown()
	{
		Memory::Delete(mVulkanInstance);
		//Shutdown();
	}

	void RenderDeviceVk::OnUpdate()
	{
		//static UInt32 currFrame = 0;
		//UInt32 nextImageIndex = 0;

		//// [CPU 2 GPU] in current frame, if the commands of current queue had not been completed, wait for it and reset its status to un-signal. 
		//WaitForFence(mpInFlightFence[currFrame]);
		//// [GPU 2 GPU]
		//AcquireNextImage(mpFetchImageSemaphore[currFrame], nextImageIndex);

		//// wait for the execution of these commands to finish using mpInFlightFence.
		//QueueSubmit(mpFetchImageSemaphore[currFrame], mpRenderFinishSemaphore[currFrame], mpInFlightFence[currFrame], mpRenderCommmands[currFrame]);
		//QueuePresent(mpRenderFinishSemaphore[currFrame], currFrame);

		//currFrame = (currFrame + 1) % SG_SWAPCHAIN_IMAGE_COUNT;
	}

	void RenderDeviceVk::OnDraw()
	{

	}

	bool RenderDeviceVk::OnSystemMessage(ESystemMessage msg)
	{
		return true;
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

		//auto* pOS = SSystem()->GetOS();
		//Window* window = pOS->GetMainWindow();
		//auto rect = window->GetCurrRect();
		//Resolution swapchainRes = { GetRectWidth(rect), GetRectHeight(rect) };
		//if (!CreateSwapchain(mSwapchain, EImageFormat::eSrgb_B8G8R8A8, EPresentMode::eFIFO, swapchainRes))
		//{
		//	SG_LOG_ERROR("Failed to create swapchain!");
		//	return false;
		//}

		//const char* stages[] = { "basic.vert", "basic.frag" };
		//CreateShader(&mpTriangleShader, stages, 2);
		//CreatePipeline(&mpDefaultPipeline, EPipelineType::eGraphic, mpTriangleShader); // here we create the render pass
		//DestroyShader(mpTriangleShader);

		//for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
		//{
		//	CreateSemaphores(&mpFetchImageSemaphore[i]);
		//	CreateSemaphores(&mpRenderFinishSemaphore[i]);
		//	CreateFence(&mpInFlightFence[i]);
		//	CreateFence(&mpImageInFlightFence[i]);
		//}

		//if (!CreateFrameBuffer(&mpFrameBuffer, mpDefaultPipeline))
		//{
		//	SG_LOG_ERROR("Failed to create framebuffer!");
		//	return false;
		//}

		//if (!CreateCommandPool(&mpCommandPool, &mGraphicQueue))
		//{
		//	SG_LOG_ERROR("Failed to create command pool!");
		//	return false;
		//}
		//for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
		//{
		//	if (!AllocateCommandBuffer(&mpRenderCommmands[i], mpCommandPool))
		//	{
		//		SG_LOG_ERROR("Failed to allocate command buffers from command pool!");
		//		return false;
		//	}
		//}

		//ResourceBarrier rtBarrier = {};
		//for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
		//{
		//	auto* pCommand = mpRenderCommmands[i];
		//	RenderTarget* pRt[] = { &mSwapchain.renderTargets[i] };
		//	BeginCommand(pCommand);

		//	rtBarrier = { pRt[0], EResoureceBarrier::efPresent, EResoureceBarrier::efRenderTarget };
		//	CmdRenderTargetResourceBarrier(pCommand, 1, &rtBarrier);

		//	CmdBindRenderTarget(pCommand, pRt, 1, i); // begin renderpass
		//		CmdSetViewport(pCommand, 0, 0, (float)pRt[0]->pTexture->resolution.width, (float)pRt[0]->pTexture->resolution.height, 0.0f, 1.0f);
		//		CmdSetScissor(pCommand, 0, 0, pRt[0]->pTexture->resolution.width, pRt[0]->pTexture->resolution.height);

		//		CmdBindPipeline(pCommand, mpDefaultPipeline);
		//		CmdDraw(pCommand, 3, 1, 0, 0);
		//	CmdBindRenderTarget(pCommand, nullptr, 0, i); // end renderpass

		//	rtBarrier = { pRt[0], EResoureceBarrier::efRenderTarget, EResoureceBarrier::efPresent };
		//	CmdRenderTargetResourceBarrier(pCommand, 1, &rtBarrier);

		//	EndCommand(pCommand);
		//}

		return true;
	}

	void RenderDeviceVk::Shutdown()
	{
		//WaitQueueIdle(&mGraphicQueue);

		//for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
		//	DestroyCommandBuffer(mpRenderCommmands[i]);

		//DestroyFrameBuffer(mpFrameBuffer);
		//for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
		//{
		//	DestroyFence(mpImageInFlightFence[i]);
		//	DestroyFence(mpInFlightFence[i]);
		//	DestroySemaphores(mpRenderFinishSemaphore[i]);
		//	DestroySemaphores(mpFetchImageSemaphore[i]);
		//}

		//DestroyCommandPool(mpCommandPool);
		//DestroyPipeline(mpDefaultPipeline);
		//DestroySwapchain(mSwapchain);
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

		swapchain.format = format;

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

		swapchain.renderTargets.resize(SG_SWAPCHAIN_IMAGE_COUNT);
		// fetch image from swapchain
		UInt32 imageCnt;
		vkGetSwapchainImagesKHR(mInstance.logicalDevice, swapchain.handle, &imageCnt, nullptr);
		vector<VkImage> vkImages;  
		vkImages.resize(imageCnt);
		vkGetSwapchainImagesKHR(mInstance.logicalDevice, swapchain.handle, &imageCnt, vkImages.data());

		for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
		{
			swapchain.renderTargets[i].pTexture = Memory::New<Texture>();
			
			swapchain.renderTargets[i].pTexture->resolution = { swapchain.extent.width, swapchain.extent.height };
			swapchain.renderTargets[i].pTexture->format = format;
			swapchain.renderTargets[i].pTexture->arrayCount = 1;
			swapchain.renderTargets[i].pTexture->image = vkImages[i];

			// create image view for swapchain image
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapchain.renderTargets[i].pTexture->image;
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

			if (vkCreateImageView(mInstance.logicalDevice, &createInfo, nullptr, &swapchain.renderTargets[i].pTexture->imageView) != VK_SUCCESS)
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
		{
			vkDestroyImageView(mInstance.logicalDevice, swapchain.renderTargets[i].pTexture->imageView, nullptr);
			Memory::Delete(swapchain.renderTargets[i].pTexture);
		}
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

			auto* pFS = SSystem()->GetFileSystem();
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
	}

	bool RenderDeviceVk::CreatePipeline(Pipeline** ppPipeline, EPipelineType type, const Shader* const pShader)
	{
		*ppPipeline = Memory::New<Pipeline>();
		Pipeline* pPipeline = *ppPipeline;

		//if (!CreatePipelineLayout(pPipeline, pShader))
		//{
		//	SG_LOG_ERROR("Failed to create pipeline layout!");
		//	return false;
		//}

		const ShaderStages& shaderStages = pShader->stages;

		vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;
		for (auto beg = shaderStages.begin(); beg != shaderStages.end(); beg++)
		{
			VkPipelineShaderStageCreateInfo shaderStageInfo = {};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			switch (beg->first)
			{
			case EShaderStages::efVert: shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
			case EShaderStages::efTesc: shaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
			case EShaderStages::efTese: shaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
			case EShaderStages::efGeom: shaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
			case EShaderStages::efFrag: shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			case EShaderStages::efComp: shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
			default: SG_LOG_ERROR("Invalid shader stage!"); SG_ASSERT(false); break;
			}
			shaderStageInfo.module = (VkShaderModule)beg->second.pShader;
			// set to main temporary
			shaderStageInfo.pName = "main";
			shaderCreateInfos.emplace_back(shaderStageInfo);
		}

		// no vertex data yet
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)mSwapchain.extent.width;
		viewport.height = (float)mSwapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { mSwapchain.extent.width, mSwapchain.extent.height };

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		// TODO: add user defined.
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// enable all dynamic states
		VkDynamicState dynamicStates[2];
		dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
		dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
		//dynamicStates[2] = VK_DYNAMIC_STATE_DEPTH_BIAS;
		//dynamicStates[3] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
		//dynamicStates[4] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
		//dynamicStates[5] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(mInstance.logicalDevice, &pipelineLayoutInfo, nullptr, &pPipeline->layout) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create pipeline layout!");
			return false;
		}

		if (!CreateRenderPass(&mpRenderPass))
		{
			SG_LOG_ERROR("Failed to create renderpass!");
			return false;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (UInt32)shaderCreateInfos.size();
		pipelineInfo.pStages = shaderCreateInfos.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState; // Optional

		pipelineInfo.layout = pPipeline->layout;

		pipelineInfo.renderPass = mpRenderPass->handle;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(mInstance.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pPipeline->handle) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create pipeline!");
			return false;
		}
		return true;
	}

	void RenderDeviceVk::DestroyPipeline(Pipeline* pPipeline)
	{
		vkDestroyPipelineLayout(mInstance.logicalDevice, pPipeline->layout, nullptr);
		vkDestroyRenderPass(mInstance.logicalDevice, mpRenderPass->handle, nullptr);
		vkDestroyPipeline(mInstance.logicalDevice, pPipeline->handle, nullptr);
		Memory::Delete(mpRenderPass); // Do do that! Remove render pass out of the pipeline.
		Memory::Delete(pPipeline);
	}

	bool RenderDeviceVk::CreateFrameBuffer(FrameBuffer** ppFrameBuffer, Pipeline* pPipeline)
	{
		*ppFrameBuffer = Memory::New<FrameBuffer>();
		auto* pFrameBuffer = *ppFrameBuffer;

		pFrameBuffer->handles.resize(SG_SWAPCHAIN_IMAGE_COUNT);

		bool bFailed = false;
		for (Size i = 0; i < mSwapchain.renderTargets.size(); i++)
		{
			VkImageView attachments[] = {
				mSwapchain.renderTargets[i].pTexture->imageView
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = mpRenderPass->handle;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width  = mSwapchain.extent.width;
			framebufferInfo.height = mSwapchain.extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(mInstance.logicalDevice, &framebufferInfo, nullptr, &pFrameBuffer->handles[i]) != VK_SUCCESS)
			{
				bFailed = true;
				if (i >= 1)
				{
					for (Size j = i - 1; j >= 0; j--) // delete previous handles
						vkDestroyFramebuffer(mInstance.logicalDevice, pFrameBuffer->handles[j], nullptr);
				}
				break;
			}
		}

		if (bFailed)
			return false;
		return true;
	}

	void RenderDeviceVk::DestroyFrameBuffer(FrameBuffer* pFrameBuffer)
	{
		for (Size i = 0; i < pFrameBuffer->handles.size(); i++)
			vkDestroyFramebuffer(mInstance.logicalDevice, pFrameBuffer->handles[i], nullptr);
		Memory::Delete(pFrameBuffer);
	}

	bool RenderDeviceVk::CreateSemaphores(Semaphore** ppSemaphore)
	{
		*ppSemaphore = Memory::New<Semaphore>();
		auto* pSemaphore = *ppSemaphore;
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(mInstance.logicalDevice, &semaphoreInfo, nullptr, &pSemaphore->handle) != VK_SUCCESS)
			return false;
		return true;
	}

	void RenderDeviceVk::DestroySemaphores(Semaphore* pSemaphore)
	{
		vkDestroySemaphore(mInstance.logicalDevice, pSemaphore->handle, nullptr);
		Memory::Delete(pSemaphore);
	}

	bool RenderDeviceVk::CreateFence(Fence** ppFence)
	{
		*ppFence = Memory::New<Fence>();
		auto* pFence = *ppFence;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(mInstance.logicalDevice, &fenceInfo, nullptr, &pFence->handle))
			return false;
		return true;
	}

	void RenderDeviceVk::DestroyFence(Fence* pFence)
	{
		vkDestroyFence(mInstance.logicalDevice, pFence->handle, nullptr);
		Memory::Delete(pFence);
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
		auto* pOS = SSystem()->GetOS();
		Window* mainWindow = pOS->GetMainWindow();

#ifdef SG_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)mainWindow->GetNativeHandle();
		createInfo.hinstance = ::GetModuleHandle(NULL);

		if (!vkCreateWin32SurfaceKHR(mInstance.instance, &createInfo, nullptr, &mInstance.presentSurface) != VK_SUCCESS)
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
		else
		{
			return false;
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
		string shaderPath = SSystem()->GetResourceDirectory(EResourceDirectory::eShader_Sources) + name + "." + extension;
		string outputPath = SSystem()->GetResourceDirectory(EResourceDirectory::eShader_Binarires) + compiledName;

		const char* args[3] = { shaderPath.c_str(), "-o", outputPath.c_str() };
		string pOut = SSystem()->GetResourceDirectory(EResourceDirectory::eShader_Binarires) + name + "-" + extension + "-compile.log";

		// create a process to use vulkanSDK to compile shader sources to binary (spirv)
		if (SSystem()->RunProcess(glslc, args, 3, pOut.c_str()) != 0)
		{
			SG_LOG_WARN("%s", pOut);
			SG_ASSERT(false);
		}

		return eastl::move(compiledName);
	}

	bool RenderDeviceVk::ReadBinaryFromDisk(Shader* pShader, const string& name, const string& extension)
	{
		auto* pFS = SSystem()->GetFileSystem();
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

	bool RenderDeviceVk::CreateRenderPass(RenderPass** ppRenderPass)
	{
		*ppRenderPass = Memory::New<RenderPass>();
		auto* pRenderPass = *ppRenderPass;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = ToVkImageFormat(mSwapchain.format);
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(mInstance.logicalDevice, &renderPassInfo, nullptr, &pRenderPass->handle) != VK_SUCCESS)
			return false;
		return true;
	}

	void RenderDeviceVk::DestroyRenderPass(RenderPass* pRenderPass)
	{
		vkDestroyRenderPass(mInstance.logicalDevice, pRenderPass->handle, nullptr);
	}

	bool RenderDeviceVk::CreateCommandPool(CommandPool** ppCommandPool, Queue* pQueue)
	{
		*ppCommandPool = Memory::New<CommandPool>();
		auto* pCommandPool = *ppCommandPool;
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = pQueue->queueFamilyIndex.value();
		poolInfo.flags = 0; // Optional

		if (vkCreateCommandPool(mInstance.logicalDevice, &poolInfo, nullptr, &pCommandPool->handle) != VK_SUCCESS)
			return false;
		return true;
	}

	void RenderDeviceVk::DestroyCommandPool(CommandPool* pCommandPool)
	{
		vkDestroyCommandPool(mInstance.logicalDevice, pCommandPool->handle, nullptr);
		Memory::Delete(pCommandPool);
	}

	bool RenderDeviceVk::AllocateCommandBuffer(RenderCommand** ppBuffers, CommandPool* pPool)
	{
		*ppBuffers = Memory::New<RenderCommand>();
		auto* pBuffer = *ppBuffers;
		pBuffer->pCurrentRenderPass = mpRenderPass;

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = pPool->handle;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(mInstance.logicalDevice, &allocInfo, &pBuffer->pCommandBuffer) != VK_SUCCESS)
			return false;
		return true;
	}

	void RenderDeviceVk::BeginCommand(RenderCommand* pBuffers)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // point to the secondary command buffer to inherit from.

		if (vkBeginCommandBuffer(pBuffers->pCommandBuffer, &beginInfo) != VK_SUCCESS)
			SG_LOG_WARN("Failed to begin command buffer!");
	}

	void RenderDeviceVk::CmdBindRenderTarget(RenderCommand* pBuffers, RenderTarget** ppRenderTargets, UInt32 numRts, UInt32 frameIndex)
	{
		if (pBuffers->bActiveRenderPass)
		{
			vkCmdEndRenderPass(pBuffers->pCommandBuffer);
			pBuffers->bActiveRenderPass = false;
		}

		if (numRts == 0) // no binding
			return;

		ResourceBarrier rtBarriers[SG_SWAPCHAIN_IMAGE_COUNT] = {};
		for (UInt32 i = 0; i < SG_SWAPCHAIN_IMAGE_COUNT; i++)
			rtBarriers[i] = { ppRenderTargets[i], EResoureceBarrier::efUndefined, EResoureceBarrier::efRenderTarget };
		CmdRenderTargetResourceBarrier(pBuffers, numRts, rtBarriers);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pBuffers->pCurrentRenderPass->handle;
		renderPassInfo.framebuffer = mpFrameBuffer->handles[frameIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = mSwapchain.extent;

		// TODO: expose to user
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(pBuffers->pCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		pBuffers->bActiveRenderPass = true;
	}

	void RenderDeviceVk::CmdBindPipeline(RenderCommand* pBuffers, Pipeline* pPipeline)
	{
		VkPipelineBindPoint bindPoint = {};
		switch (pPipeline->type)
		{
			case EPipelineType::eGraphic: bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; break;
			case EPipelineType::eCompute: bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE; break;
			case EPipelineType::eTransfer: bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE; break;
		}
		vkCmdBindPipeline(pBuffers->pCommandBuffer, bindPoint, pPipeline->handle);
	}

	void RenderDeviceVk::CmdDraw(RenderCommand* pBuffers, UInt32 vertexCnt, UInt32 instanceCnt, UInt32 firstVertex, UInt32 firstInstance)
	{
		vkCmdDraw(pBuffers->pCommandBuffer, vertexCnt, instanceCnt, firstVertex, firstInstance);
	}

	void RenderDeviceVk::CmdRenderTargetResourceBarrier(RenderCommand* pBuffers, UInt32 numRTs, ResourceBarrier* ppResourceBarries)
	{
		VkAccessFlags srcAccessFlags = 0;
		VkAccessFlags dstAccessFlags = 0;

		vector<VkImageMemoryBarrier> pImageBarriers;
		for (uint32_t i = 0; i < numRTs; ++i)
		{
			ResourceBarrier& currentBarrier = ppResourceBarries[i];
			RenderTarget* pRt = eastl::get<RenderTarget*>(ppResourceBarries[i].pResource);
			VkImageMemoryBarrier pImageBarrier = {};

			if (EResoureceBarrier::efUnordered_Access == currentBarrier.oldState &&
				EResoureceBarrier::efUnordered_Access == currentBarrier.newState)
			{
				pImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				pImageBarrier.pNext = nullptr;

				pImageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				pImageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
				pImageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
				pImageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			}
			else
			{
				pImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				pImageBarrier.pNext = nullptr;

				pImageBarrier.srcAccessMask = ToVkAccessFlags(currentBarrier.oldState);
				pImageBarrier.dstAccessMask = ToVkAccessFlags(currentBarrier.newState);
				// when the rt of the swapchain been used first, use VK_IMAGE_LAYOUT_UNDEFINED. 
				pImageBarrier.oldLayout = pRt->bUsed ? ToVkImageLayout(currentBarrier.oldState) : VK_IMAGE_LAYOUT_UNDEFINED;
				if (!pRt->bUsed) pRt->bUsed = true;
				pImageBarrier.newLayout = ToVkImageLayout(currentBarrier.newState);
			}

			pImageBarrier.image = pRt->pTexture->image;
			pImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO: change it if it is DepthRT
			pImageBarrier.subresourceRange.baseMipLevel = 0;
			pImageBarrier.subresourceRange.levelCount = 1;

			pImageBarrier.subresourceRange.baseArrayLayer = 0;
			pImageBarrier.subresourceRange.layerCount = 1;

			pImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			pImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			srcAccessFlags |= pImageBarrier.srcAccessMask;
			dstAccessFlags |= pImageBarrier.dstAccessMask;

			pImageBarriers.emplace_back(pImageBarrier);
		}

		VkPipelineStageFlags srcStageMask = ToVkPipelineStageFlags(srcAccessFlags, EQueueType::eGraphic);
		VkPipelineStageFlags dstStageMask = ToVkPipelineStageFlags(dstAccessFlags, EQueueType::eGraphic);

		if (numRTs)
		{
			vkCmdPipelineBarrier(pBuffers->pCommandBuffer, srcStageMask, dstStageMask, 0,
				0, nullptr, // we don't use memory barrier now
				0, nullptr,
				numRTs, pImageBarriers.data());
		}
	}

	void RenderDeviceVk::CmdSetViewport(RenderCommand* pBuffers, float xOffset, float yOffset, float width, float height, float minDepth, float maxDepth)
	{
		VkViewport viewport = {};
		viewport.x = xOffset;
		viewport.y = yOffset;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;
		vkCmdSetViewport(pBuffers->pCommandBuffer, 0, 1, &viewport);
	}

	void RenderDeviceVk::CmdSetScissor(RenderCommand* pBuffers, UInt32 xOffset, UInt32 yOffset, UInt32 width, UInt32 height)
	{
		VkRect2D scissor = {};
		scissor.offset.x = xOffset;
		scissor.offset.y = yOffset;
		scissor.extent.width = width;
		scissor.extent.height = height;
		vkCmdSetScissor(pBuffers->pCommandBuffer, 0, 1, &scissor);
	}

	void RenderDeviceVk::EndCommand(RenderCommand* pBuffers)
	{
		if (pBuffers->bActiveRenderPass)
		{
			vkCmdEndRenderPass(pBuffers->pCommandBuffer);
			pBuffers->bActiveRenderPass = false;
		}
		if (vkEndCommandBuffer(pBuffers->pCommandBuffer) != VK_SUCCESS)
			SG_LOG_WARN("Failed to end command buffers!");
	}

	void RenderDeviceVk::DestroyCommandBuffer(RenderCommand* pBuffers)
	{
		Memory::Delete(pBuffers);
	}

	void RenderDeviceVk::AcquireNextImage(Semaphore* pWaitSemaphore, UInt32& imageIndex, UInt64 timeOut)
	{
		vkAcquireNextImageKHR(mInstance.logicalDevice, mSwapchain.handle, timeOut, pWaitSemaphore->handle, VK_NULL_HANDLE, &imageIndex);
	}

	void RenderDeviceVk::QueueSubmit(Semaphore* pWaitSemaphore, Semaphore* pSignalSemaphore, Fence* pCPU2GPUCommandExecutedFence, RenderCommand* pCommand)
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { pWaitSemaphore->handle };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT /*, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT */ };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &pCommand->pCommandBuffer;

		VkSemaphore signalSemaphores[] = { pSignalSemaphore->handle };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(mGraphicQueue.handle, 1, &submitInfo, pCPU2GPUCommandExecutedFence->handle))
			SG_LOG_ERROR("Failed to submit render commands to queue!");
	}

	void RenderDeviceVk::QueuePresent(Semaphore* pWaitSemaphore, UInt32 frameIndex)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &pWaitSemaphore->handle;

		VkSwapchainKHR swapChains[] = { mSwapchain.handle };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &frameIndex;
		presentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(mGraphicQueue.handle, &presentInfo);
	}

	void RenderDeviceVk::WaitQueueIdle(Queue* pQueue)
	{
		vkQueueWaitIdle(pQueue->handle);
	}

	void RenderDeviceVk::WaitForFence(Fence* pFence, UInt64 timeOut /*= UINT64_MAX*/)
	{
		vkWaitForFences(mInstance.logicalDevice, 1, &pFence->handle, VK_TRUE, UINT64_MAX);
		vkResetFences(mInstance.logicalDevice, 1, &pFence->handle);
	}

}