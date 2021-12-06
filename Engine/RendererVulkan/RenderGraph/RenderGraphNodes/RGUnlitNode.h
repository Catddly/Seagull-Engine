#pragma once

#include "Render/FrameBuffer.h"

#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanDevice;

	class VulkanRenderTarget;
	class VulkanPipeline;
	class VulkanPipelineLayout;

	class RGUnlitNode final : public RenderGraphNode
	{
	public:
		RGUnlitNode(VulkanDevice& device);
		~RGUnlitNode() = default;

		void BindMainRenderTarget(VulkanRenderTarget* pColorRt, const LoadStoreClearOp& op);
		void BindMainDepthBuffer(VulkanRenderTarget* pDepthRt, const LoadStoreClearOp& op);

		void BindGeometry(const char* name);
		void BindPipeline(VulkanPipelineLayout* pLayout, Shader* pShader);
		void AddDescriptorSet(UInt32 set, VkDescriptorSet handle);

		void AddConstantBuffer(EShaderStage stage, UInt32 size, void* pData);
	private:
		virtual VulkanRenderPass* Prepare() override;
		virtual void Execute(VulkanCommandBuffer& pBuf) override;
		virtual void Clear() override;

		VulkanDevice&         mDevice;
		VulkanRenderPass*     mpRenderPass;

		VulkanRenderTarget*   mpColorRt;
		LoadStoreClearOp      mColorRtLoadStoreOp;
		VulkanRenderTarget*   mpDepthRt;
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