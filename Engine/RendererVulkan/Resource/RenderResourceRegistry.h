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

	class Geometry;

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
		SG_RENDERER_VK_API VulkanBuffer* GetBuffer(const string& name) const;
		void DeleteBuffer(const string& name);
		void FlushBuffers() const;

		//bool CreateGeometry(const char* name, const vector<Vector3f>& vertices, const vector<UInt32>& indices);
		//bool CreateGeometry(const char* name, const vector<Vector3f>& vertices, const vector<UInt16>& indices);

		bool CreateGeometry(const char* name, const float* pVerticies, const UInt32 numVertex, const UInt32* pIndices, const UInt32 numIndex);
		bool CreateGeometry(const char* name, const float* pVerticies, const UInt32 numVertex, const UInt16* pIndices, const UInt32 numIndex);
		SG_RENDERER_VK_API Geometry* GetGeometry(const string& name) const;

		bool HaveBuffer(const char* name);
		bool UpdataBufferData(const char* name, const void* pData);

		bool CreateTexture(const TextureCreateDesc& textureCI, bool bLocal = false);
		SG_RENDERER_VK_API VulkanTexture* GetTexture(const string& name) const;
		void FlushTextures() const;

		bool CreateSampler(const SamplerCreateDesc& samplerCI);
		SG_RENDERER_VK_API VulkanSampler* GetSampler(const string& name) const;

		SG_RENDERER_VK_API static VulkanResourceRegistry* GetInstance();
	private:
		VulkanResourceRegistry() = default;
	private:
		VulkanContext* mpContext;
		mutable eastl::unordered_map<string, VulkanBuffer*>  mBuffers;
		mutable eastl::unordered_map<string, VulkanTexture*> mTextures;
		mutable eastl::unordered_map<string, VulkanSampler*> mSamplers;
		mutable eastl::unordered_map<string, Geometry*>      mGeometries;

		mutable vector<eastl::pair<BufferCreateDesc, VulkanBuffer*>>  mWaitToSubmitBuffers;
		mutable vector<eastl::pair<BufferCreateDesc, VulkanTexture*>> mWaitToSubmitTextures;
	};

	// for convenience
#define VK_RESOURCE() VulkanResourceRegistry::GetInstance()

}