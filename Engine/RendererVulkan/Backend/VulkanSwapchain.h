#pragma once

#include "Base/BasicTypes.h"
#include "RendererVulkan/Config.h"
#include "Render/SwapChain.h"
#include "RendererVulkan/Utils/VkConvert.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"

#include <vulkan/vulkan_core.h>

#include "Stl/vector.h"

namespace SG
{

	class VulkanQueue;
	class VulkanSemaphore;

	// TODO: abstract to IResource
	class VulkanRenderTarget : public RenderTarget
	{
	public:
		VulkanRenderTarget(VulkanDevice& d) : device(d) {}
		VulkanRenderTarget(VulkanDevice& d, const RenderTargetCreateDesc& CI);
		~VulkanRenderTarget();

		virtual UInt32 GetWidth()     const override { return width; };
		virtual UInt32 GetHeight()    const override { return height; };
		virtual UInt32 GetDepth()     const override { return depth; };
		virtual UInt32 GetNumArray()  const override { return array; };
		virtual UInt32 GetNumMipmap() const override { return mipmap; };

		virtual EImageFormat GetFormat() const { return ToSGImageFormat(format); }
		virtual ESampleCount GetSample() const { return ToSGSampleCount(sample); }
		virtual EImageType   GetType()   const { return ToSGImageType(type); }
		virtual EImageUsage  GetUsage()  const { return ToSGImageUsage(usage); }

		static VulkanRenderTarget* Create(VulkanDevice& d, const RenderTargetCreateDesc& CI);
	private:
		friend class VulkanSwapchain;
	public:
		VulkanDevice&         device;

		VkImage               image;
		VkImageView           imageView;
		VkDeviceMemory        memory;

		UInt32                width;
		UInt32                height;
		UInt32                depth;
		UInt32                array;
		UInt32                mipmap;
		VkFormat              format;
		VkImageType           type;
		VkSampleCountFlagBits sample;
		VkImageUsageFlags     usage;
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
		VulkanSwapchain(VulkanInstance& instance, VulkanDevice& device);
		~VulkanSwapchain();

		VkSwapchainKHR       swapchain = VK_NULL_HANDLE;
		UInt32               imageCount;
		vector<VkImage>      images;
		vector<VkImageView>  imageViews;

		bool CreateOrRecreate(UInt32 width, UInt32 height, bool vsync = false);
		VulkanRenderTarget* GetRenderTarget(UInt32 index) const;
		void CleanUp();

		bool AcquireNextImage(VulkanSemaphore* signalSemaphore, UInt32& imageIndex);
		EImageState Present(VulkanQueue* queue, UInt32 imageIndex, VulkanSemaphore* signalSemaphore);
	private:
		bool CreateSurface();
		void DestroySurface();
		bool CheckSurfacePresentable(UInt32 familyIndex);
	private:
		VulkanInstance&   mInstance;
		VulkanDevice&     mDevice;
		bool              bSwapchainAdequate = false;

		VkSurfaceKHR      mPresentSurface;
		VkFormat          mFormat;
		VkColorSpaceKHR   mColorSpace;

		vector<VulkanRenderTarget*> mpRts;
	};

}