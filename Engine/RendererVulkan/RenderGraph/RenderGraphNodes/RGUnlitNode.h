#pragma once

#include "Render/FrameBuffer.h"

#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanContext;

	class VulkanRenderTarget;
	class VulkanPipeline;
	class VulkanPipelineLayout;

	class RGUnlitNode final : public RenderGraphNode
	{
	public:
		RGUnlitNode(VulkanContext& context);
		~RGUnlitNode() = default;

		void SetMainColorRTClearOp(const LoadStoreClearOp& op) { mColorRtLoadStoreOp = op; }
		void SetDepthRTClearOp(const LoadStoreClearOp& op) { mDepthRtLoadStoreOp = op; }

		void BindGeometry(const char* name);
		void BindPipeline(VulkanPipelineLayout* pLayout, Shader* pShader);
		void AddDescriptorSet(UInt32 set, VkDescriptorSet handle);

		void AddConstantBuffer(EShaderStage stage, UInt32 size, void* pData);
	private:
		virtual VulkanRenderPass* Prepare() override;
		virtual void Execute(VulkanCommandBuffer& pBuf) override;
		virtual void Clear() override;

		VulkanContext&        mContext;
		VulkanRenderPass*     mpRenderPass;

		LoadStoreClearOp      mColorRtLoadStoreOp;
		LoadStoreClearOp      mDepthRtLoadStoreOp;

		VulkanPipeline*       mpPipeline;
		VulkanPipelineLayout* mpPipelineLayout;
		Shader*               mpShader;

		Geometry*		      mpGeometry;
		
		vector<eastl::pair<UInt32, VkDescriptorSet>> mDescriptorSets;
		struct BindConstantData
		{
			EShaderStage stage;
			UInt32       size;
			void*        pData;

			BindConstantData(EShaderStage s, UInt32 sz, void* ptr)
				:stage(s), size(sz), pData(ptr)
			{}
		};
		vector<BindConstantData> mPushConstants;
	};

}