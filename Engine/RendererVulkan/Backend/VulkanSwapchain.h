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

	class VulkanSampler
	{
	public:
		VulkanSampler(VulkanDevice& d, const SamplerCreateDesc& CI);
		~VulkanSampler();

		static VulkanSampler* Create(VulkanDevice& d, const SamplerCreateDesc& CI);
	private:
		friend class VulkanDescriptorDataBinder;

		VulkanDevice& device;
		VkSampler     sampler;
	};

	class VulkanTexture
	{
	public:
		VulkanTexture(VulkanDevice& d, const TextureCreateDesc& CI, bool bLocal = false);
		~VulkanTexture();

		static VulkanTexture* Create(VulkanDevice& d, const TextureCreateDesc& CI, bool bLocal = false);

		UInt32 GetWidth()  const { return width; }
		UInt32 GetHeight() const { return height; }
		UInt32 GetDepth()     const { return depth; };
		UInt32 GetNumArray()  const { return array; };
		UInt32 GetNumMipmap() const { return mipLevel; };

		EImageFormat GetFormat() const { return format; }
		ESampleCount GetSample() const { return sample; }
		EImageType   GetType()   const { return type; }
		EImageUsage  GetUsage()  const { return usage; }
	private:
		friend class VulkanCommandBuffer;
		friend class VulkanDescriptorDataBinder;

		VulkanDevice&  device;
		VkImage        image;
		VkDeviceMemory memory;
		VkImageView    imageView;
		VkImageLayout  currLayout; // used to do some safety check

		UInt32 width;
		UInt32 height;
		UInt32 depth;
		UInt32 mipLevel;
		UInt32 array;

		EImageType   type;
		EImageFormat format;
		ESampleCount sample;
		EImageUsage  usage;
	};

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
		friend class VulkanFrameBuffer;
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