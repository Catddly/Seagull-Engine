#pragma once

#include "Base/BasicTypes.h"
#include "Render/SwapChain.h"
#include "Archive/IDAllocator.h"

#include "VulkanAllocator.h"

#include "volk.h"

namespace SG
{

	class VulkanDevice;

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

	class VulkanContext;

	class VulkanTexture
	{
	public:
		VulkanTexture(VulkanContext& c) : context(c) {}
		VulkanTexture(VulkanContext& c, const TextureCreateDesc& CI);
		~VulkanTexture();

		static VulkanTexture* Create(VulkanContext& c, const TextureCreateDesc& CI);

		UInt32 GetWidth()  const { return width; }
		UInt32 GetHeight() const { return height; }
		UInt32 GetDepth()     const { return depth; };
		UInt32 GetNumArray()  const { return array; };
		UInt32 GetNumMipmap() const { return mipLevel; };

		EImageFormat GetFormat() const { return format; }
		ESampleCount GetSample() const { return sample; }
		EImageType   GetType()   const { return type; }
		EImageUsage  GetUsage()  const { return usage; }

		// TODO: make a complete id system
		UInt32       GetID()     const { return id; }

		void* GetUserData() const { return pUserData; }
	protected:
		friend class VulkanCommandBuffer;
		friend class VulkanDescriptorDataBinder;
		VulkanContext& context;

		VkImage        image;
		VkImageView    imageView;
#if SG_USE_VULKAN_MEMORY_ALLOCATOR
		VmaAllocation  vmaAllocation;
#else
		VkDeviceMemory memory;
#endif
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

		UInt32 id;
		void* pUserData;

		static IDAllocator<UInt32> msIdAllocator;
	};

	// TODO: abstract to IResource
	class VulkanRenderTarget final : public VulkanTexture
	{
	public:
		VulkanRenderTarget(VulkanContext& c, bool isDepth = false)
			: VulkanTexture(c), mbIsDepth(isDepth)
		{}
		VulkanRenderTarget(VulkanContext& c, const TextureCreateDesc& CI, bool isDepth = false)
			: VulkanTexture(c, CI), mbIsDepth(isDepth)
		{}

		bool IsDepth() const { return mbIsDepth; }

		static VulkanRenderTarget* Create(VulkanContext& c, const TextureCreateDesc& CI, bool isDepth = false);
	private:
		friend class VulkanSwapchain;
		friend class VulkanFrameBuffer;
		bool mbIsDepth;
	};

}