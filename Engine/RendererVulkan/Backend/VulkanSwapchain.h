#pragma once

#include "Base/BasicTypes.h"
#include "RendererVulkan/Config.h"
#include "Render/SwapChain.h"
#include "RendererVulkan/Utils/VkConvert.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"

namespace SG
{

	struct VulkanQueue;
	struct VulkanSemaphore;

	// TODO: abstract to IResource
	struct VulkanRenderTarget : public RenderTarget
	{
		VkImage               image;
		VkImageView           imageView;
		VkDeviceMemory        memory;

		VkFormat              format;
		VkImageType           type;
		VkSampleCountFlagBits sample;
		VkImageUsageFlags     usage;
		UInt32                width;
		UInt32                height;
		UInt32                depth;
		UInt32                array;
		UInt32                mipmap;

		virtual UInt32 GetWidth()     const override { return width; };
		virtual UInt32 GetHeight()    const override { return height; };
		virtual UInt32 GetDepth()     const override { return depth; };
		virtual UInt32 GetNumArray()  const override { return array; };
		virtual UInt32 GetNumMipmap() const override { return mipmap; };

		virtual EImageFormat       GetFormat() const { return ToSGImageFormat(format); }
		virtual ESampleCount       GetSample() const { return ToSGSampleCount(sample); }
		virtual EImageType         GetType()   const { return ToSGImageType(type); }
		virtual ERenderTargetUsage GetUsage()  const { return ToSGImageUsage(usage); }
	};

	struct VulkanFrameBuffer
	{
		vector<VkFramebuffer>       frameBuffers;
		//VkRenderPass                renderPass;
		//vector<VulkanRenderTarget*> renderTargets;
	};

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VkInstance instance);
		~VulkanSwapchain();

		VkSwapchainKHR       swapchain = VK_NULL_HANDLE;
		UInt32               imageCount;
		vector<VkImage>      images;
		vector<VkImageView>  imageViews;

		void BindDevice(VkPhysicalDevice physicalDevice, VkDevice device);

		bool CreateOrRecreate(UInt32 width, UInt32 height, bool vsync = false);
		void Destroy();

		VulkanRenderTarget* GetRenderTarget(UInt32 index) const;
		bool AcquireNextImage(VulkanSemaphore* signalSemaphore, UInt32& imageIndex);
		EImageState Present(VulkanQueue* queue, UInt32 imageIndex, VulkanSemaphore* signalSemaphore);

		bool CreateSurface();
		bool CheckSurfacePresentable(VulkanQueue* queue);
	private:
		VkInstance       mInstance;
		VkPhysicalDevice mPhysicalDevice;
		VkDevice	     mLogicalDevice;
		bool             bSwapchainAdequate = false;

		VkSurfaceKHR     mPresentSurface;
		VkFormat         mFormat;
		VkColorSpaceKHR  mColorSpace;

		vector<VulkanRenderTarget*> mpRts;
	};

}