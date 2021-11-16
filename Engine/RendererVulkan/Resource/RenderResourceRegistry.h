#pragma once

#include "RendererVulkan/Config.h"
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

	class VulkanResourceRegistry
	{
	public:
		~VulkanResourceRegistry() = default;

		void Initialize(const VulkanContext* pContext);
		void Shutdown();

		void FlushBuffers() const;

		// By default, create the buffer using HOST_VISIBLE bit.
		bool CreateBuffer(const BufferCreateDesc& bufferCI, bool bLocal = false);
		//bool CreateRenderTarget();

		SG_RENDERER_VK_API VulkanBuffer* GetBuffer(const char* name);
		bool          HaveBuffer(const char* name);
		bool          UpdataBufferData(const char* name, void* pData);

		SG_RENDERER_VK_API static VulkanResourceRegistry* GetInstance();
	private:
		VulkanResourceRegistry() = default;
	private:
		VulkanContext* mpContext;
		eastl::unordered_map<const char*, VulkanRenderTarget*> mRenderTargets;
		eastl::unordered_map<const char*, VulkanBuffer*> mBuffers;
		eastl::unordered_map<const char*, VulkanPipelineDataSet> mPipelines;

		mutable vector<eastl::pair<BufferCreateDesc, VulkanBuffer*>> mWaitToSubmitBuffers;
	};

}