#pragma once

#include "RendererVulkan/Config.h"
#include "Render/Buffer.h"
#include "Render/Shader.h"
#include "Render/SwapChain.h"

#include "Stl/vector.h"
#include <eastl/utility.h>
#include <eastl/unordered_map.h>
#include <eastl/hash_map.h>

namespace SG
{
	// TODO: resource object reference counting
	class VulkanContext;

	class VulkanRenderTarget;
	class VulkanTexture;
	class VulkanSampler;
	class VulkanBuffer;

	class VulkanPipeline;
	class VulkanPipelineSetLayout;

	class VulkanDescriptorSet;

	struct VulkanPipelineDataSet
	{
		VulkanPipeline*          pipeline;
		VulkanPipelineSetLayout* pipelineLayout;
		Shader*                  shader;

		eastl::unordered_map<UInt32, VulkanDescriptorSet*> descriptorSetsMap;
	};

	class VulkanResourceRegistry
	{
	public:
		~VulkanResourceRegistry() = default;

		void Initialize(const VulkanContext* pContext);
		void Shutdown();

		// By default, create the buffer using HOST_VISIBLE bit.
		bool CreateBuffer(const BufferCreateDesc& bufferCI, bool bLocal = false);
		SG_RENDERER_VK_API VulkanBuffer* GetBuffer(const char* name);
		void FlushBuffers() const;

		bool HaveBuffer(const char* name);
		bool UpdataBufferData(const char* name, void* pData);

		bool CreateTexture(const TextureCreateDesc& textureCI, bool bLocal = false);
		SG_RENDERER_VK_API VulkanTexture* GetTexture(const char* name);
		void FlushTextures() const;

		bool CreateSampler(const SamplerCreateDesc& samplerCI);
		SG_RENDERER_VK_API VulkanSampler* GetSampler(const char* name);

		SG_RENDERER_VK_API static VulkanResourceRegistry* GetInstance();
	private:
		VulkanResourceRegistry() = default;
	private:
		VulkanContext* mpContext;
		eastl::unordered_map<const char*, VulkanRenderTarget*> mRenderTargets;
		eastl::unordered_map<const char*, VulkanBuffer*>       mBuffers;
		eastl::unordered_map<const char*, VulkanTexture*>      mTextures;
		eastl::unordered_map<const char*, VulkanSampler*>      mSamplers;
		eastl::unordered_map<const char*, VulkanPipelineDataSet> mPipelines;

		mutable vector<eastl::pair<BufferCreateDesc, VulkanBuffer*>>  mWaitToSubmitBuffers;
		mutable vector<eastl::pair<BufferCreateDesc, VulkanTexture*>> mWaitToSubmitTextures;
	};

	// for convenience
#define VK_RESOURCE() VulkanResourceRegistry::GetInstance()

}