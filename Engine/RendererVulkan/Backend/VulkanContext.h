#pragma once

#include "Defs/Defs.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanDescriptorPool;
	class VulkanCommandPool;

	class VulkanContext
	{
	public:
		VulkanContext();
		~VulkanContext();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanContext);

		VulkanInstance  instance;
		VulkanSwapchain swapchain;
		VulkanDevice    device;

		VulkanCommandPool* graphicCommandPool;
		VulkanCommandPool* computeCommandPool;
		VulkanCommandPool* transferCommandPool;
		VkRenderPass       defaultRenderPass;

		VulkanDescriptorPool* pDefaultDescriptorPool;

		// should be moved to other place
		VkDescriptorSet cameraUBOSet;

		vector<VulkanRenderTarget*> colorRts;
		VulkanRenderTarget*         depthRt;

		void WindowResize();
	private:
		void CreateDefaultResource();
		void DestroyDefaultResource();
	};

}