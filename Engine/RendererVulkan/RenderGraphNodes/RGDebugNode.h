#pragma once

#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanContext;
	class VulkanShader;
	class VulkanPipeline;
	class VulkanPipelineSignature;

	class RGDebugNode final : public RenderGraphNode
	{
	public:
		RGDebugNode(VulkanContext& context, RenderGraph* pRenderGraph);
		~RGDebugNode();
	private:
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Draw(DrawInfo& context) override;
	private:
		VulkanContext& mContext;

		LoadStoreClearOp mColorRtLoadStoreOp;
		LoadStoreClearOp mDepthRtLoadStoreOp;

		Matrix4f mDebugObjectModelMat;

		RefPtr<VulkanPipelineSignature> mpDebugLinePipelineSignature;
		VulkanPipeline*                 mpDebugLinePipeline;
		RefPtr<VulkanShader>            mpDebugLineShader;
	};

}