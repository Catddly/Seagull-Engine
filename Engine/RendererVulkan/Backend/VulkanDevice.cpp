#include "StdAfx.h"
#include "VulkanDevice.h"

#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include "VulkanInstance.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderContext.h"
#include "VulkanSynchronizePrimitive.h"
#include "RendererVulkan/Utils/VkConvert.h"

#include "RendererVulkan/Backend/VulkanRenderContext.h"

#include "Stl/vector.h"
#include <eastl/array.h>

namespace SG
{

	VulkanDevice::VulkanDevice(VkPhysicalDevice device)
		:physicalDevice(device)
	{
		// get all the extensions supported by device
		UInt32 extCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			supportedExtensions.resize(extCount);
			vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, supportedExtensions.data());
		}

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
	}

	VulkanDevice::~VulkanDevice()
	{
		DestroyDescriptorPool(defaultDescriptorPool);
		if (graphicCommandPool != VK_NULL_HANDLE)
			DestroyCommandPool(graphicCommandPool);
		if (transferCommandPool != VK_NULL_HANDLE && queueFamilyIndices.graphics != queueFamilyIndices.transfer)
			DestroyCommandPool(transferCommandPool);
		if (computeCommandPool != VK_NULL_HANDLE && queueFamilyIndices.graphics != queueFamilyIndices.compute)
			DestroyCommandPool(computeCommandPool);
		if (logicalDevice != VK_NULL_HANDLE)
			DestroyLogicalDevice();
	}

	void VulkanDevice::WaitIdle() const
	{
		vkDeviceWaitIdle(logicalDevice);
	}

	bool VulkanDevice::CreateLogicalDevice(void* pNext)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

		const float DEFAULT_QUEUE_PRIORITY = 0.0f;
		// graphic queue
		int graphics = FetchQueueFamilyIndicies(VK_QUEUE_GRAPHICS_BIT);
		if (graphics != -1)
		{
			queueFamilyIndices.graphics = graphics;
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &DEFAULT_QUEUE_PRIORITY;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.graphics = VK_NULL_HANDLE;
		}

		// compute queue
		int compute = FetchQueueFamilyIndicies(VK_QUEUE_COMPUTE_BIT);
		if (compute != -1 && compute != graphics)
		{
			queueFamilyIndices.compute = compute;
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &DEFAULT_QUEUE_PRIORITY;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.compute = queueFamilyIndices.graphics;
		}

		// transfer queue
		int transfer = FetchQueueFamilyIndicies(VK_QUEUE_TRANSFER_BIT);
		if (transfer != -1 && transfer != graphics)
		{
			queueFamilyIndices.transfer = transfer;
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &DEFAULT_QUEUE_PRIORITY;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.transfer = queueFamilyIndices.graphics;
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		// to support swapchain
		vector<const char*> deviceExtensions = {};
		if (SupportExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		if (SupportExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
			deviceExtensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = (UInt32)queueCreateInfos.size();
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = (UInt32)deviceExtensions.size();;
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		createInfo.enabledLayerCount = 0;
#ifdef SG_ENABLE_VK_VALIDATION_LAYER
		vector<const char*> layers;
		layers.emplace_back("VK_LAYER_KHRONOS_validation");

		createInfo.enabledLayerCount = (UInt32)layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
#endif
		
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
		if (pNext) 
		{
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.features = deviceFeatures;
			physicalDeviceFeatures2.pNext = pNext;
			createInfo.pEnabledFeatures = nullptr;
			createInfo.pNext = &physicalDeviceFeatures2;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create logical device!");
			return false;
		}

		// create a default command pool to allocate commands to graphic queue.
		graphicCommandPool = CreateCommandPool(queueFamilyIndices.graphics);
		if (graphicCommandPool == VK_NULL_HANDLE)
		{
			SG_LOG_ERROR("Failed to create default graphic command pool!");
			return false;
		}

		// create a default command pool to allocate commands to transfer queue.
		if (queueFamilyIndices.transfer == queueFamilyIndices.graphics)
		{
			transferCommandPool = graphicCommandPool;
		}
		else
		{
			transferCommandPool = CreateCommandPool(queueFamilyIndices.transfer);
			if (transferCommandPool == VK_NULL_HANDLE)
			{
				SG_LOG_ERROR("Failed to create default transfer command pool!");
				return false;
			}
		}

		// create a default command pool to allocate commands to compute queue.
		if (queueFamilyIndices.compute == queueFamilyIndices.graphics)
		{
			computeCommandPool = graphicCommandPool;
		}
		else
		{
			computeCommandPool = CreateCommandPool(queueFamilyIndices.compute);
			if (computeCommandPool == VK_NULL_HANDLE)
			{
				SG_LOG_ERROR("Failed to create default compute command pool!");
				return false;
			}
		}

		defaultDescriptorPool = CreateDescriptorPool();

		return true;
	}

	void VulkanDevice::DestroyLogicalDevice()
	{
		vkDestroyDevice(logicalDevice, nullptr);
	}

	VkCommandPool VulkanDevice::CreateCommandPool(UInt32 queueFamilyIndices, VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndices;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;
		if (vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &cmdPool) != VK_SUCCESS)
			return VK_NULL_HANDLE;
		return cmdPool;
	}

	void VulkanDevice::DestroyCommandPool(VkCommandPool pool)
	{
		vkDestroyCommandPool(logicalDevice, pool, nullptr);
	}

	VkSemaphore VulkanDevice::CreateSemaphore()
	{
		VkSemaphore semaphore;
		VkSemaphoreCreateInfo semaphoreCI = {};
		semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCI.pNext = nullptr;

		if (vkCreateSemaphore(logicalDevice, &semaphoreCI, nullptr, &semaphore) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create vulkan semaphore!");
			return VK_NULL_HANDLE;
		}
		return semaphore;
	}

	void VulkanDevice::DestroySemaphore(VkSemaphore semaphore)
	{
		vkDestroySemaphore(logicalDevice, semaphore, nullptr);
	}

	VkFence VulkanDevice::CreateFence(bool bSignaled)
	{
		VkFence fence;
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// create in signaled state so we don't wait on first render of each command buffer.
		if (bSignaled)
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if (vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create vulkan fence");
			return VK_NULL_HANDLE;
		}
		return fence;
	}

	void VulkanDevice::DestroyFence(VkFence fence)
	{
		vkDestroyFence(logicalDevice, fence, nullptr);
	}

	void VulkanDevice::ResetFence(VkFence fence)
	{
		vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);
		vkResetFences(logicalDevice, 1, &fence);
	}

	SG::VulkanRenderContext* VulkanDevice::CreateRenderContext(UInt32 numBuffers, EQueueType type)
	{
		auto* pRenderContext = Memory::New<VulkanRenderContext>(numBuffers);
		switch (type)
		{
		case SG::EQueueType::eGraphic:  pRenderContext->commandPool = graphicCommandPool; break;
		case SG::EQueueType::eTransfer: pRenderContext->commandPool = transferCommandPool; break;
		case SG::EQueueType::eCompute:  pRenderContext->commandPool = computeCommandPool; break;
		case SG::EQueueType::eNull:
		case SG::EQueueType::MAX_COUNT:
		default: SG_LOG_ERROR("Invalid queue type to create render context!"); break;
		}

		AllocateCommandBuffers(pRenderContext);
		return pRenderContext;
	}

	void VulkanDevice::DestroyRenderContext(VulkanRenderContext* pContext)
	{
		FreeCommandBuffers(pContext);
		Memory::Delete(pContext);
	}

	bool VulkanDevice::AllocateCommandBuffers(VulkanRenderContext* pContext)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = pContext->commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = (UInt32)pContext->commandBuffers.size();
		
		if (vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, pContext->commandBuffers.data()) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to allocate command buffer");
			return false;
		}
		return true;
	}

	void VulkanDevice::FreeCommandBuffers(VulkanRenderContext* pContext)
	{
		vkFreeCommandBuffers(logicalDevice, pContext->commandPool, (UInt32)pContext->commandBuffers.size(), pContext->commandBuffers.data());
	}

	VkFramebuffer VulkanDevice::CreateFrameBuffer(VkRenderPass renderPass, VulkanRenderTarget* pColorRt, VulkanRenderTarget* pDepthRt)
	{
		if (!pColorRt)
		{
			SG_LOG_WARN("pColorRt should not be nullptr!");
			return VK_NULL_HANDLE;
		}

		VkFramebuffer fb;
		UInt32 numRts = 1;
		if (pDepthRt)
			++numRts;

		eastl::array<VkImageView, 2> attachments;
		attachments[0] = pColorRt->imageView;
		if (pDepthRt)
			attachments[1] = pDepthRt->imageView;

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		frameBufferCreateInfo.renderPass = renderPass;
		frameBufferCreateInfo.attachmentCount = numRts;
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width  = pColorRt->width;
		frameBufferCreateInfo.height = pColorRt->height;
		frameBufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(logicalDevice, &frameBufferCreateInfo, nullptr, &fb) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create vulkan frame buffer!");
			return VK_NULL_HANDLE;
		}
		return fb;
	}

	void VulkanDevice::DestroyFrameBuffer(VkFramebuffer frameBuffer)
	{
		vkDestroyFramebuffer(logicalDevice, frameBuffer, nullptr);
	}

	VulkanRenderTarget* VulkanDevice::CreateRenderTarget(const RenderTargetCreateDesc& rt)
	{
		// TODO: use resource system to store the resource
		VulkanRenderTarget* renderTarget = Memory::New<VulkanRenderTarget>();
		renderTarget->width  = rt.width;
		renderTarget->height = rt.height;
		renderTarget->depth  = rt.depth;
		renderTarget->mipmap = rt.mipmap;
		renderTarget->array  = rt.array;

		renderTarget->format = ToVkImageFormat(rt.format);
		renderTarget->type   = ToVkImageType(rt.type);
		renderTarget->sample = ToVkSampleCount(rt.sample);
		renderTarget->usage  = ToVkImageUsage(rt.usage);

		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = renderTarget->type;
		imageCI.format = renderTarget->format;
		imageCI.extent = { rt.width, rt.height, rt.depth };
		imageCI.mipLevels = rt.mipmap;
		imageCI.arrayLayers = rt.array;
		imageCI.samples = renderTarget->sample;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = renderTarget->usage;

		VK_CHECK(vkCreateImage(logicalDevice, &imageCI, nullptr, &renderTarget->image),
			SG_LOG_ERROR("Failed to create render targets' image!"); Memory::Delete(renderTarget); return false;);

		VkMemoryRequirements memReqs = {};
		vkGetImageMemoryRequirements(logicalDevice, renderTarget->image, &memReqs);

		VkMemoryAllocateInfo memAllloc = {};
		memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllloc.allocationSize = memReqs.size;
		memAllloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkAllocateMemory(logicalDevice, &memAllloc, nullptr, &renderTarget->memory);
		vkBindImageMemory(logicalDevice, renderTarget->image, renderTarget->memory, 0);

		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.viewType = ToVkImageViewType(renderTarget->type, renderTarget->array);
		imageViewCI.image = renderTarget->image;
		imageViewCI.format = renderTarget->format;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = renderTarget->array;

		if (renderTarget->usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			// stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
			if (renderTarget->format >= VK_FORMAT_D16_UNORM_S8_UINT) {
				imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

		VK_CHECK(vkCreateImageView(logicalDevice, &imageViewCI, nullptr, &renderTarget->imageView),
			SG_LOG_ERROR("Failed to create render targets' image view!"); Memory::Delete(renderTarget); return nullptr;);

		return renderTarget;
	}

	void VulkanDevice::DestroyRenderTarget(VulkanRenderTarget* rt)
	{
		vkDestroyImageView(logicalDevice, rt->imageView, nullptr);
		vkDestroyImage(logicalDevice, rt->image, nullptr);
		vkFreeMemory(logicalDevice, rt->memory, nullptr);
	}

	VkRenderPass VulkanDevice::CreateRenderPass(VulkanRenderTarget* pColorRt, VulkanRenderTarget* pDepthRt)
	{
		VkRenderPass renderPass;
		eastl::array<VkAttachmentDescription, 2> attachments = {};

		if (pColorRt)
		{
			attachments[0].format = pColorRt->format;
			attachments[0].samples = pColorRt->sample;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Keep its contents after the render pass is finished (for displaying it)
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		if (pDepthRt)
		{
			attachments[1].format = pDepthRt->format;                 
			attachments[1].samples = pDepthRt->sample;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // We don't need depth after render pass has finished (DONT_CARE may result in better performance)
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;                                    // Attachment 0 is color
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Attachment layout used as color during the subpass

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;                                            // Attachment 1 is depth
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Attachment used as depth/stencil used during the subpass

		// Setup a single subpass reference
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.inputAttachmentCount = 0;       // Input attachments can be used to sample from contents of a previous subpass
		subpassDescription.pInputAttachments = nullptr;    // (Input attachments not used by this example)
		subpassDescription.preserveAttachmentCount = 0;    // Preserved attachments can be used to loop (and preserve) attachments through subpasses
		subpassDescription.pPreserveAttachments = nullptr; // (Preserve attachments not used by this example)
		subpassDescription.pResolveAttachments = nullptr;  // Resolve attachments are resolved at the end of a sub pass and can be used for e.g. multi sampling

		// Setup subpass dependencies
		// These will add the implicit attachment layout transitions specified by the attachment descriptions
		// The actual usage layout is preserved through the layout specified in the attachment reference
		// Each subpass dependency will introduce a memory and execution dependency between the source and dest subpass described by
		// srcStageMask, dstStageMask, srcAccessMask, dstAccessMask (and dependencyFlags is set)
		// Note: VK_SUBPASS_EXTERNAL is a special constant that refers to all commands executed outside of the actual renderpass)
		eastl::array<VkSubpassDependency, 2> dependencies = {};

		// First dependency at the start of the renderpass
		// Does the transition from final to initial layout
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;                             // Producer of the dependency
		dependencies[0].dstSubpass = 0;                                               // Consumer is our single subpass that will wait for the execution dependency
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Match our pWaitDstStageMask when we vkQueueSubmit
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a loadOp stage for color attachments
		dependencies[0].srcAccessMask = 0;                                            // semaphore wait already does memory dependency for us
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a loadOp CLEAR access mask for color attachments
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Second dependency at the end the renderpass
		// Does the transition from the initial to the final layout
		// Technically this is the same as the implicit subpass dependency, but we are gonna state it explicitly here
		dependencies[1].srcSubpass = 0;                                               // Producer of the dependency is our single subpass
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;                             // Consumer are all commands outside of the renderpass
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a storeOp stage for color attachments
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;          // Do not block any subsequent work
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a storeOp `STORE` access mask for color attachments
		dependencies[1].dstAccessMask = 0;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create renderPass!");
			return VK_NULL_HANDLE;
		}
		return renderPass;
	}

	void VulkanDevice::DestroyRenderPass(VkRenderPass renderPass)
	{
		vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
	}

	VkPipelineCache VulkanDevice::CreatePipelineCache()
	{
		VkPipelineCache pipelineCache;
		VkPipelineCacheCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if (vkCreatePipelineCache(logicalDevice, &createInfo, nullptr, &pipelineCache) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create pipeline cache!");
			return VK_NULL_HANDLE;
		}
		return pipelineCache;
	}

	void VulkanDevice::DestroyPipelineCache(VkPipelineCache pipelineCache)
	{
		vkDestroyPipelineCache(logicalDevice, pipelineCache, nullptr);
	}

	VkPipelineLayout VulkanDevice::CreatePipelineLayout(VulkanBuffer* pBuffer)
	{
		// bind descriptors layout to pipeline
		VkDescriptorSetLayout descriptorSetLayout;
		if (pBuffer)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // TODO: change it dynamically
			layoutBinding.binding = 0;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			layoutBinding.pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
			descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorLayout.pNext = nullptr;
			descriptorLayout.bindingCount = 1;
			descriptorLayout.pBindings    = &layoutBinding;

			VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout),
				SG_LOG_ERROR("Failed to create descriptor set layout!"); return VK_NULL_HANDLE; );
			pBuffer->descriptorSetLayout = descriptorSetLayout;
		}

		VkPipelineLayout layout;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = pBuffer ? 1 : 0;
		pipelineLayoutInfo.pSetLayouts = pBuffer ? &descriptorSetLayout : nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VK_CHECK(vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &layout),
			SG_LOG_ERROR("Failed to create pipeline layout!"); return VK_NULL_HANDLE; );

		return layout;
	}

	void VulkanDevice::DestroyPipelineLayout(VkPipelineLayout layout)
	{
		vkDestroyPipelineLayout(logicalDevice, layout, nullptr);
	}

	VkPipeline VulkanDevice::CreatePipeline(VkPipelineCache pipelineCache, VkPipelineLayout layout, VkRenderPass renderPass, Shader& shader, BufferLayout* pLayout)
	{
		VkPipeline pipeline;
		vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
		for (auto beg = shader.begin(); beg != shader.end(); ++beg)
		{
			VkPipelineShaderStageCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			switch (beg->first)
			{
			case EShaderStages::efVert:	createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
			case EShaderStages::efTesc:	createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
			case EShaderStages::efTese:	createInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
			case EShaderStages::efGeom:	createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
			case EShaderStages::efFrag:	createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			case EShaderStages::efComp:	createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
			default:                    SG_LOG_ERROR("Unknown type of shader stage!"); break;
			}
			
			VkShaderModule shaderModule;
			VkShaderModuleCreateInfo moduleCreateInfo = {};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = beg->second.binarySize;
			moduleCreateInfo.pCode = (UInt32*)beg->second.pBinary;
			vkCreateShaderModule(logicalDevice, &moduleCreateInfo, nullptr, &shaderModule);

			Memory::Free(beg->second.pBinary);

			createInfo.module = shaderModule;
			createInfo.pName = "main";
			shaderStages.push_back(createInfo);
		}
		shader.clear();

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = layout;
		pipelineCreateInfo.renderPass = renderPass;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
		blendAttachmentState[0].colorWriteMask = 0xf;
		blendAttachmentState[0].blendEnable = VK_FALSE;
		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = blendAttachmentState;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkDynamicState dynamicStateEnables[2] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 2;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.front = depthStencilState.back;

		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.pSampleMask = nullptr;

		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = pLayout->GetTotalSize();
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		vector<VkVertexInputAttributeDescription> vertexInputAttributs;
		UInt32 loc = 0;
		for (auto& e : *pLayout)
		{
			VkVertexInputAttributeDescription vertexInputAttrib = {};
			vertexInputAttrib.binding  = 0;
			vertexInputAttrib.location = loc;
			vertexInputAttrib.format = ToVkShaderDataFormat(e.type);
			vertexInputAttrib.offset = e.offset;
			++loc;
			vertexInputAttributs.emplace_back(vertexInputAttrib);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputState = {};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = (UInt32)vertexInputAttributs.size();
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

		// assign the pipeline states to the pipeline creation info structure
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.pDynamicState = &dynamicState;

		if (vkCreateGraphicsPipelines(logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create graphics pipeline!");
			return VK_NULL_HANDLE;
		}

		for (UInt32 i = 0; i < shaderStages.size(); ++i)
			vkDestroyShaderModule(logicalDevice, shaderStages[i].module, nullptr);
		return pipeline;
	}

	void VulkanDevice::DestroyPipeline(VkPipeline pipeline)
	{
		vkDestroyPipeline(logicalDevice, pipeline, nullptr);
	}

	VulkanQueue* VulkanDevice::GetQueue(EQueueType type) const
	{
		VulkanQueue* queue = Memory::New<VulkanQueue>();
		switch (type)
		{
		case SG::EQueueType::eNull: SG_LOG_ERROR("Wrong queue type!"); break;
		case SG::EQueueType::eGraphic:  
			vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphics, 0, &queue->handle); 
			queue->familyIndex = queueFamilyIndices.graphics;
			break;
		case SG::EQueueType::eCompute:
			vkGetDeviceQueue(logicalDevice, queueFamilyIndices.compute, 0, &queue->handle);
			queue->familyIndex = queueFamilyIndices.compute;
			break;
		case SG::EQueueType::eTransfer: 
			vkGetDeviceQueue(logicalDevice, queueFamilyIndices.transfer, 0, &queue->handle);
			queue->familyIndex = queueFamilyIndices.transfer;
			break;
		default: SG_LOG_ERROR("Unknown queue type!"); break;
		}
		queue->type = type;
		queue->priority = EQueuePriority::eNormal;
		return queue;
	}

	SG::VulkanBuffer* VulkanDevice::CreateBuffer(const BufferCreateDesc& bufferCI)
	{
		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;

		VulkanBuffer       stagingBuffer = {};
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		if (SG_HAS_ENUM_FLAG(bufferCI.type, EBufferType::efVertex) || SG_HAS_ENUM_FLAG(bufferCI.type, EBufferType::efIndex))
		{
			// staging buffer creation
			bufferInfo.size = bufferCI.totalSizeInByte;
			stagingBuffer.totalSizeInByte = bufferCI.totalSizeInByte;
			stagingBuffer.device = logicalDevice;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // used to copy data

			VK_CHECK(vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &stagingBuffer.buffer),
				SG_LOG_ERROR("Failed to create staging buffer!"); return nullptr; );
			vkGetBufferMemoryRequirements(logicalDevice, stagingBuffer.buffer, &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			VK_CHECK(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &stagingBuffer.memory),
				SG_LOG_ERROR("Failed to alloc memory for staging buffer!"); return nullptr;);

			stagingBuffer.UploadData(bufferCI.pData);
			VK_CHECK(vkBindBufferMemory(logicalDevice, stagingBuffer.buffer, stagingBuffer.memory, 0),
				SG_LOG_ERROR("Failed to bind buffer memory!"); return nullptr; );
		}

		// buffer creation
		auto* pBuffer = Memory::New<VulkanBuffer>();
		pBuffer->totalSizeInByte = bufferCI.totalSizeInByte;
		pBuffer->device = logicalDevice;
		pBuffer->type   = bufferCI.type;

		bufferInfo.size = bufferCI.totalSizeInByte;
		bufferInfo.usage = ToVkBufferUsage(bufferCI.type);
		VK_CHECK(vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &pBuffer->buffer),
			SG_LOG_ERROR("Failed to create vulkan buffer!"); Memory::Delete(pBuffer); return nullptr; );
		vkGetBufferMemoryRequirements(logicalDevice, pBuffer->buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		if (SG_HAS_ENUM_FLAG(bufferCI.type, EBufferType::efVertex) || SG_HAS_ENUM_FLAG(bufferCI.type, EBufferType::efIndex))
			memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // device-local buffer
		else
			memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &pBuffer->memory), 
			SG_LOG_ERROR("Failed to alloc memory for buffer!"); Memory::Delete(pBuffer); return nullptr; );

		// TODO: support memory offset.
		VK_CHECK(vkBindBufferMemory(logicalDevice, pBuffer->buffer, pBuffer->memory, 0), 
			SG_LOG_ERROR("Failed to bind vulkan memory to buffer!"); Memory::Delete(pBuffer); return false;);

		if (SG_HAS_ENUM_FLAG(bufferCI.type, EBufferType::efVertex) || SG_HAS_ENUM_FLAG(bufferCI.type, EBufferType::efIndex))
		{
			// add copy commands to transfer queue
			// TODO: not to submit once at one buffer, but submit them together.
			VulkanRenderContext context(1);
			context.commandPool = transferCommandPool;
			AllocateCommandBuffers(&context);

			context.CmdBeginCommandBuf(context.commandBuffers[0]);
			context.CmdCopyBuffer(context.commandBuffers[0], stagingBuffer.buffer, pBuffer->buffer, bufferCI.totalSizeInByte);
			context.CmdEndCommandBuf(context.commandBuffers[0]);

			VulkanQueue* pTransferQueue = GetQueue(EQueueType::eTransfer);
			VulkanFence  waitFence;
			waitFence.fence = CreateFence();
			pTransferQueue->SubmitCommands(&context, 0, nullptr, nullptr, &waitFence);
			ResetFence(waitFence.fence);

			DestroyFence(waitFence.fence);
			FreeCommandBuffers(&context);

			DestroyBuffer(&stagingBuffer);
		}

		pBuffer->descriptor.buffer = pBuffer->buffer;
		pBuffer->descriptor.offset = 0;
		pBuffer->descriptor.range = bufferCI.totalSizeInByte;

		return pBuffer;
	}

	void VulkanDevice::DestroyBuffer(VulkanBuffer* pBuffer)
	{
		vkDestroyDescriptorSetLayout(logicalDevice, pBuffer->descriptorSetLayout, nullptr);
		vkFreeMemory(logicalDevice, pBuffer->memory, nullptr);
		vkDestroyBuffer(logicalDevice, pBuffer->buffer, nullptr);
	}

	VkDescriptorPool VulkanDevice::CreateDescriptorPool()
	{
		VkDescriptorPool pool;
		VkDescriptorPoolSize typeCounts[1];
		typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		typeCounts[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.poolSizeCount = 1;
		descriptorPoolInfo.pPoolSizes = typeCounts;
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK(vkCreateDescriptorPool(logicalDevice, &descriptorPoolInfo, nullptr, &pool),
			SG_LOG_ERROR("Failed to create descriptor pool!"); return VK_NULL_HANDLE; );
		return pool;
	}

	void VulkanDevice::DestroyDescriptorPool(VkDescriptorPool pool)
	{
		vkDestroyDescriptorPool(logicalDevice, pool, nullptr);
	}

	VkDescriptorSet VulkanDevice::AllocateDescriptorSet(VkDescriptorSetLayout layout)
	{
		VkDescriptorSet descriptorSet;
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = defaultDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VK_CHECK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet),
			SG_LOG_ERROR("Failed to allocate descriptor set!"); return VK_NULL_HANDLE; );
		return descriptorSet;
	}

	void VulkanDevice::FreeDescriptorSet(VkDescriptorSet set)
	{
		vkFreeDescriptorSets(logicalDevice, defaultDescriptorPool, 1, &set);
	}

	bool VulkanDevice::SupportExtension(const string& extension)
	{
		for (auto beg = supportedExtensions.begin(); beg != supportedExtensions.end(); beg++)
		{
			if (beg->extensionName == extension)
				return true;
		}
		return false;
	}

	int VulkanDevice::FetchQueueFamilyIndicies(VkQueueFlagBits flags)
	{
		// find a compute queue
		UInt32 i = 0;
		if (flags & VK_QUEUE_COMPUTE_BIT)
		{
			for (const auto& e : queueFamilyProperties)
			{
				if ((e.queueFlags & flags) && ((e.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) // have COMPUTE_BIT but bot have the GRAPHIC_BIT
				{
					return i;
				}
				++i;
			}
		}

		// find a transfer queue
		i = 0;
		if (flags & VK_QUEUE_TRANSFER_BIT)
		{
			for (const auto& e : queueFamilyProperties)
			{
				if ((e.queueFlags & flags) && ((e.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
					&& ((e.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) // have TRANSFER_BIT but bot have the GRAPHIC_BIT
				{
					return i;
				}
				++i;
			}
		}

		// find a graphics queue
		i = 0;
		for (const auto& e : queueFamilyProperties)
		{
			if (e.queueFlags & flags) // have TRANSFER_BIT but bot have the GRAPHIC_BIT
			{
				return i;
			}
			++i;
		}

		return -1;
	}

	SG::UInt32 VulkanDevice::GetMemoryType(UInt32 typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (memTypeFound)
					{
						*memTypeFound = true;
					}
					return i;
				}
			}
			typeBits >>= 1;
		}

		if (memTypeFound)
		{
			*memTypeFound = false;
			return 0;
		}
		else
		{
			SG_LOG_ERROR("Failed to find device memory type!");
			SG_ASSERT(false);
		}

		return 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// VulkanQueue
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bool VulkanQueue::SubmitCommands(RenderContext* pContext, UInt32 bufferIndex, RenderSemaphore* renderSemaphore, RenderSemaphore* presentSemaphore, RenderFence* fence)
	{
		auto* pVkContext   = static_cast<VulkanRenderContext*>(pContext);
		auto* pVkFence     = static_cast<VulkanFence*>(fence);
		auto* pVkRenderSP  = static_cast<VulkanSemaphore*>(renderSemaphore);
		auto* pVkPresentSP = static_cast<VulkanSemaphore*>(presentSemaphore);

		// pipeline stage at which the queue submission will wait (via pWaitSemaphores)

		// the submit info structure specifies a command buffer queue submission batch
		VkPipelineStageFlags waitStageMask = {};
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		if (renderSemaphore && presentSemaphore)
		{
			waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		submitInfo.pWaitDstStageMask = &waitStageMask;

		if (presentSemaphore)
		{
			submitInfo.pWaitSemaphores = &pVkPresentSP->semaphore;
			submitInfo.waitSemaphoreCount = 1;
		}
		if (renderSemaphore)
		{
			submitInfo.pSignalSemaphores = &pVkRenderSP->semaphore;
			submitInfo.signalSemaphoreCount = 1;
		}
		submitInfo.pCommandBuffers = &pVkContext->commandBuffers[bufferIndex];
		submitInfo.commandBufferCount = 1;

		if (vkQueueSubmit(handle, 1, &submitInfo, pVkFence->fence) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to submit render commands to queue!");
			return false;
		}
		return true;
	}

	void VulkanQueue::WaitIdle() const
	{
		vkQueueWaitIdle(handle);
	}

}