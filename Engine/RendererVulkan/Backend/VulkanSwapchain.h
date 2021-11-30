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
		VulkanTexture(VulkanDevice& d) : device(d) {}
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
	protected:
		friend class VulkanCommandBuffer;
		friend class VulkanDescriptorDataBinder;
		VulkanDevice&  device;

		VkImage        image;
		VkImageView    imageView;
		VkDeviceMemory memory;
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
	class VulkanRenderTarget final : public VulkanTexture
	{
	public:
		VulkanRenderTarget(VulkanDevice& d) : VulkanTexture(d) {}
		VulkanRenderTarget(VulkanDevice& d, const TextureCreateDesc& CI) : VulkanTexture(d, CI, true) {}

		static VulkanRenderTarget* Create(VulkanDevice& d, const TextureCreateDesc& CI);
	private:
		friend class VulkanSwapchain;
		friend class VulkanFrameBuffer;
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