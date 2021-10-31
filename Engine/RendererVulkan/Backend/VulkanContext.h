#pragma once

#include "Defs/Defs.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanContext
	{
	public:
		VulkanContext();
		~VulkanContext();
		SG_CLASS_NO_COPY_ASSIGNABLE(VulkanContext);

		VulkanInstance  instance;
		VulkanSwapchain swapchain;
		VulkanDevice    device;

		VkCommandPool    graphicCommandPool;
		VkCommandPool    computeCommandPool;
		VkCommandPool    transferCommandPool;
		VkRenderPass     defaultRenderPass;

		VkDescriptorPool defaultDescriptorPool;

		vector<VulkanRenderTarget*> colorRts;
		VulkanRenderTarget*         depthRt;

		void WindowResize();
	private:
		void CreateDefaultResource();
		void DestroyDefaultResource();
	};

}