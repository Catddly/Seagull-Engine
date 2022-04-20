#pragma once

#include "Base/BasicTypes.h"
#include "RendererVulkan/Config.h"
#include "Render/SwapChain.h"
#include "RendererVulkan/Utils/VkConvert.h"

#include "RendererVulkan/RenderGraph/RenderGraphResource.h"

#include "volk.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanContext;
	class VulkanQueue;
	class VulkanSemaphore;
	class VulkanRenderTarget;

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VulkanContext& context);
		~VulkanSwapchain();

		VkSwapchainKHR       swapchain = VK_NULL_HANDLE;
		UInt32               imageCount;
		vector<VkImage>      images;
		vector<VkImageView>  imageViews;

		bool CreateOrRecreate(UInt32 width, UInt32 height, bool vsync = false);
		VulkanRenderTarget* GetRenderTarget(UInt32 index) const;
		void CleanUp();

		bool AcquireNextImage(VulkanSemaphore* pSignalSemaphore, UInt32& imageIndex);
		EImageState Present(VulkanQueue* queue, UInt32 imageIndex, VulkanSemaphore* pWaitSemaphore);
	private:
		bool CreateSurface();
		void DestroySurface();
		bool CheckSurfacePresentable(UInt32 familyIndex);
	private:
		VulkanContext&    context;
		bool              bSwapchainAdequate = false;

		VkSurfaceKHR      mPresentSurface;
		VkFormat          mFormat;
		VkColorSpaceKHR   mColorSpace;

		vector<VulkanRenderTarget*> mpRts;
	};

}