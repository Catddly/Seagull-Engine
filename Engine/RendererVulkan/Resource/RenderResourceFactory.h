#pragma once

#include "Render/Buffer.h"
#include "Render/Shader.h"

#include "Stl/vector.h"
#include <eastl/utility.h>
#include <eastl/unordered_map.h>
#include <eastl/hash_map.h>

namespace SG
{
	// TODO: resource object reference counting
	class VulkanContext;

	class VulkanRenderTarget;
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

	class VulkanResourceFactory
	{
	public:
		~VulkanResourceFactory() = default;

		void Initialize(const VulkanContext* pContext);
		void Shutdown();

		void FlushBuffers() const;

		// By default, create the buffer using HOST_VISIBLE bit.
		bool CreateBuffer(const BufferCreateDesc& bufferCI, bool bLocal = false);
		bool CreateRenderTarget();

		VulkanBuffer* GetBuffer(const char* name);
		bool          UpdataBufferData(const char* name, void* pData);

		static VulkanResourceFactory* GetInstance();
	private:
		VulkanResourceFactory() = default;
	private:
		VulkanContext* mpContext;
		eastl::unordered_map<const char*, VulkanRenderTarget*> mRenderTargets;
		eastl::unordered_map<const char*, VulkanBuffer*> mBuffers;
		eastl::unordered_map<const char*, VulkanPipelineDataSet> mPipelines;

		mutable vector<eastl::pair<BufferCreateDesc, VulkanBuffer*>> mWaitToSubmitBuffers;
	};

}