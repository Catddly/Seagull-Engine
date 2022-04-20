#pragma once

#include "Base/Handle.h"
#include "RendererVulkan/Config.h"
#include "Render/Buffer.h"
#include "Render/SwapChain.h"
#include "Scene/Scene.h"
#include "Scene/RenderDataBuilder.h"

#include "RendererVulkan/Resource/DrawCall.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanDescriptor.h"

#include "Stl/vector.h"
#include <eastl/utility.h>
#include <eastl/unordered_map.h>
#include <eastl/hash_map.h>

namespace SG
{

#define SG_MAX_PACKED_VERTEX_BUFFER_SIZE 1024 * 1024 * 4 // 4mb
#define SG_MAX_PACKED_INSTANCE_BUFFER_SIZE 1024 * 1024   // 1mb
#define SG_MAX_PACKED_INDEX_BUFFER_SIZE  1024 * 1024 * 4 // 4mb

#define SG_MAX_NUM_OBJECT 250

	// TODO: resource object reference counting
	class VulkanContext;
	class VulkanFence;

	class VulkanRenderTarget;
	class VulkanTexture;
	class VulkanSampler;
	class VulkanBuffer;

	class Shader;

	class VulkanResourceRegistry
	{
	public:
		~VulkanResourceRegistry() = default;

		void Initialize(const VulkanContext* pContext);
		void Shutdown();

		void OnUpdate();
		void WindowResize();

		const DrawCall& GetSkyboxDrawCall() const { return mSkyboxDrawCall; }

		void WaitBuffersUpdate() const;

		/// Buffer Begin
		// By default, create the buffer using HOST_VISIBLE bit.
		bool CreateBuffer(const BufferCreateDesc& bufferCI);
		VulkanBuffer* GetBuffer(const string& name) const;
		bool HaveBuffer(const char* name);
		void DeleteBuffer(const string& name);
		bool UpdataBufferData(const char* name, const void* pData);
		void FlushBuffers() const;
		/// Buffer End

		/// Texture Begin
		bool CreateTexture(const TextureCreateDesc& textureCI, bool bLocal = false);
		VulkanTexture* GetTexture(const string& name) const;
		void FlushTextures() const;
		/// Texture End

		/// DescriptorSet Begin
		void AddDescriptorSet(const string& name, VulkanDescriptorSet* pSet, bool bCreateHandle = false);
		void AddDescriptorSetHandle(const string& name, VulkanDescriptorSet* pSet);
		void RemoveDescriptorSet(const string& name);
		void RemoveDescriptorSet(VulkanDescriptorSet* pSet);
		VulkanDescriptorSet* GetDescriptorSet(const string& name) const;
		Handle<VulkanDescriptorSet>* GetDescriptorSetHandle(const string& name);
		ReadOnlyHandle<VulkanDescriptorSet> GetDescriptorSetReadOnlyHandle(const string& name);
		/// DescriptorSet End

		/// RenderTarget Begin
		bool CreateRenderTarget(const TextureCreateDesc& textureCI, bool isDepth = false);
		void DeleteRenderTarget(const string& name);
		VulkanRenderTarget* GetRenderTarget(const string& name) const;
		/// RenderTarget End

		/// Sampler Begin
		bool CreateSampler(const SamplerCreateDesc& samplerCI);
		VulkanSampler* GetSampler(const string& name) const;
		/// Sampler End

		static VulkanResourceRegistry* GetInstance();
	private:
		VulkanResourceRegistry() = default;
	private:
		VulkanContext* mpContext;
		mutable eastl::unordered_map<string, VulkanBuffer*>  mBuffers;
		mutable eastl::unordered_map<string, VulkanTexture*> mTextures;
		mutable eastl::unordered_map<string, VulkanDescriptorSet*> mDescriptorSets;
		mutable eastl::unordered_map<string, Handle<VulkanDescriptorSet>> mDescriptorSetHandles;
		mutable eastl::unordered_map<string, VulkanRenderTarget*> mRenderTargets;
		mutable eastl::unordered_map<string, VulkanSampler*> mSamplers;

		mutable vector<VulkanCommandBuffer> mSubmitedCommandBuffers; //! Cache all the transfer command buffer, clear it when do sync.
		mutable vector<VulkanBuffer*> mpStagingBuffers; //! Cache all the transfer command buffer, clear it when do sync.
		mutable vector<VulkanFence*>  mpBufferUploadFences; //! Used to sync all the resource update.
		mutable vector<eastl::pair<BufferCreateDesc, VulkanBuffer*>>  mWaitToSubmitBuffers;
		mutable vector<eastl::pair<BufferCreateDesc, VulkanTexture*>> mWaitToSubmitTextures;

		DrawCall mSkyboxDrawCall;
	};

	// for convenience
#define VK_RESOURCE() VulkanResourceRegistry::GetInstance()

}