#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "Platform/IOperatingSystem.h"
#include "Platform/Window.h"
#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include "Render/SwapChain.h"

#include "RendererVulkan/Backend/VulkanInstance.h"
#include "RendererVulkan/Backend/VulkanRenderContext.h"

namespace SG
{

	VulkanRenderDevice::VulkanRenderDevice()
	{
	}

	VulkanRenderDevice::~VulkanRenderDevice()
	{
	}

	void VulkanRenderDevice::OnInit()
	{
		mpInstance = Memory::New<VulkanInstance>();
		mpSwapchain = Memory::New<VulkanSwapchain>(mpInstance->instance);
		mpSwapchain->CreateSurface();

		if (!SelectPhysicalDeviceAndCreateDevice())
		{
			SG_LOG_ERROR("No suittable GPU detected!");
			SG_ASSERT(false);
		}
		mpDevice->CreateLogicalDevice(nullptr);
		VulkanQueue mGraphicsQueue = mpDevice->GetQueue(EQueueType::eGraphic);

		mpSwapchain->BindDevice(mpDevice->physicalDevice, mpDevice->logicalDevice);
		mpSwapchain->CheckSurfacePresentable(mGraphicsQueue);

		Window* mainWindow = SSystem()->GetOS()->GetMainWindow();
		Rect rect = mainWindow->GetCurrRect();
		mpSwapchain->CreateOrRecreate(GetRectWidth(rect), GetRectHeight(rect));

		// create command buffer
		//mCommandBuffers.resize(mSwapchain->imageCount);
		mpDevice->AllocateCommandBuffers(mpRenderContext->commandBuffers);

		// create depth stencil rt
		RenderTargetCreateDesc depthRtCI;
		depthRtCI.width  = mColorRts[0].width;
		depthRtCI.height = mColorRts[0].height;
		depthRtCI.depth  = mColorRts[0].depth;
		depthRtCI.array = 1;
		depthRtCI.format = EImageFormat::eUnorm_D24_uint_S8;
		depthRtCI.sample = ESampleCount::eSample_1;
		depthRtCI.type = EImageType::e2D;
		depthRtCI.usage = ERenderTargetUsage::eDepth_Stencil;
		mDevice->CreateRenderTarget(depthRtCI);

		ShaderStages basicShader;
		mShaderCompiler.LoadSPIRVShader("basic", basicShader);

		mDefaultRenderPass = mDevice->CreateRenderPass(&mColorRts[0], &mDepthRt);
		mPipelineCache = mDevice->CreatePipelineCache();
		mPipeline = mDevice->CreatePipeline(mPipelineCache, mDefaultRenderPass, basicShader);

		SG_LOG_DEBUG("RenderDevice - Vulkan Init");
	}

	void VulkanRenderDevice::OnShutdown()
	{
		mDevice->DestroyPipeline(mPipeline);
		mDevice->DestroyPipelineCache(mPipelineCache);
		mDevice->DestroyRenderPass(mDefaultRenderPass);

		mDevice->DestroyRenderTarget(&mDepthRt);
		mDevice->FreeCommandBuffers(mCommandBuffers);

		mSwapchain->Destroy();
		Memory::Delete(mSwapchain);
		Memory::Delete(mDevice);

		Memory::Delete(mpInstance);
		SG_LOG_DEBUG("RenderDevice - Vulkan Shutdown");
	}

	void VulkanRenderDevice::OnUpdate()
	{

	}

	void VulkanRenderDevice::OnDraw()
	{

	}

	bool VulkanRenderDevice::SelectPhysicalDeviceAndCreateDevice()
	{
		UInt32 gpuCount;
		vkEnumeratePhysicalDevices(mpInstance->instance, &gpuCount, nullptr);
		vector<VkPhysicalDevice> devices(gpuCount);
		vkEnumeratePhysicalDevices(mpInstance->instance, &gpuCount, devices.data());

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
				mpDevice = Memory::New<VulkanDevice>(device);
				return true;
			}

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			{
				mpDevice = Memory::New<VulkanDevice>(device);
				SG_LOG_WARN("Integrated GPU detected!");
				return true;
			}
		}

		return false;
	}

}