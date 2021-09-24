#include "StdAfx.h"
#include "VulkanRenderDevice.h"

#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include "RendererVulkan/Backend/VulkanInstance.h"

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
		mInstance = Memory::New<VulkanInstance>();
		SG_LOG_DEBUG("RenderDevice - Vulkan Init");
	}

	void VulkanRenderDevice::OnShutdown()
	{
		Memory::Delete(mInstance);
		SG_LOG_DEBUG("RenderDevice - Vulkan Shutdown");
	}

	void VulkanRenderDevice::OnUpdate()
	{

	}

	void VulkanRenderDevice::OnDraw()
	{

	}

}