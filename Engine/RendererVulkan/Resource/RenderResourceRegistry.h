#pragma once

#include "RendererVulkan/Config.h"
#include "Render/Buffer.h"
#include "Render/SwapChain.h"
#include "Scene/Scene.h"
#include "Scene/RenderDataBuilder.h"

#include "RendererVulkan/Resource/RenderMesh.h"

#include "Stl/vector.h"
#include <eastl/utility.h>
#include <eastl/unordered_map.h>
#include <eastl/hash_map.h>

namespace SG
{

#define SG_MAX_PACKED_VERTEX_BUFFER_SIZE 1024 * 1024 * 4 // 4mb
#define SG_MAX_PACKED_INDEX_BUFFER_SIZE  1024 * 1024 * 4 // 4mb

#define SG_MAX_NUM_OBJECT 100

	// TODO: resource object reference counting
	class VulkanContext;

	class VulkanRenderTarget;
	class VulkanTexture;
	class VulkanSampler;
	class VulkanBuffer;

	class VulkanPipeline;
	class VulkanPipelineSetLayout;

	class Shader;
	class Mesh;

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

		void OnUpdate(WeakRefPtr<Scene> pScene);

		void BuildRenderMesh(RefPtr<RenderDataBuilder> renderDataBuilder);

		const RenderMesh& GetSkyboxRenderMesh() const { return mSkyboxRenderMesh; }
		template <typename Func>
		void TraverseStaticRenderMesh(Func&& func);
		template <typename Func>
		void TraverseStaticRenderMeshInstanced(Func&& func);

		/// Buffer Begin
		// By default, create the buffer using HOST_VISIBLE bit.
		bool CreateBuffer(const BufferCreateDesc& bufferCI, bool bLocal = false);
		VulkanBuffer* GetBuffer(const string& name) const;
		bool HaveBuffer(const char* name);
		void DeleteBuffer(const string& name);
		bool UpdataBufferData(const char* name, const void* pData);
		void FlushBuffers() const;
		/// Buffer Begin

		/// Texture Begin
		bool CreateTexture(const TextureCreateDesc& textureCI, bool bLocal = false);
		VulkanTexture* GetTexture(const string& name) const;
		void FlushTextures() const;
		/// Texture Begin

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
		void CreateInnerResource();
		void DestroyInnerResource();
	private:
		VulkanContext* mpContext;
		mutable eastl::unordered_map<string, VulkanBuffer*>  mBuffers;
		mutable eastl::unordered_map<string, VulkanTexture*> mTextures;
		mutable eastl::unordered_map<string, VulkanRenderTarget*> mRenderTargets;
		mutable eastl::unordered_map<string, VulkanSampler*> mSamplers;

		// instead of use unordered_map, now packed all the vertex data into one big vertex data and give offset to them.
		// temporary
		VulkanBuffer* mPackedVertexBuffer = nullptr;
		UInt64        mPackedVBCurrOffset = 0;
		VulkanBuffer* mPackedIndexBuffer = nullptr;
		UInt64        mPackedIBCurrOffset = 0;

		RenderMesh mSkyboxRenderMesh;
		eastl::unordered_map<UInt32, RenderMesh> mStaticRenderMeshes; // Forward Mesh Pass
		eastl::unordered_map<UInt32, RenderMesh> mStaticRenderMeshesInstanced; // Forward Instance Mesh Pass

		mutable vector<eastl::pair<BufferCreateDesc, VulkanBuffer*>>  mWaitToSubmitBuffers;
		mutable vector<eastl::pair<BufferCreateDesc, VulkanTexture*>> mWaitToSubmitTextures;
	};

	template <typename Func>
	void VulkanResourceRegistry::TraverseStaticRenderMesh(Func&& func)
	{
		for (auto node : mStaticRenderMeshes)
			func(node.second);
	}

	template <typename Func>
	void VulkanResourceRegistry::TraverseStaticRenderMeshInstanced(Func&& func)
	{
		for (auto node : mStaticRenderMeshesInstanced)
			func(node.second);
	}

	// for convenience
#define VK_RESOURCE() VulkanResourceRegistry::GetInstance()

}