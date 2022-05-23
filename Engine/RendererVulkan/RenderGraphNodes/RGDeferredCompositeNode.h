#pragma once

#include "RendererVulkan/Config.h"
#include "Render/FrameBuffer.h"

#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "Stl/SmartPtr.h"

#if SG_ENABLE_DEFERRED_SHADING
namespace SG
{

	class VulkanContext;
	class VulkanRenderPass;

	class VulkanPipelineSignature;
	class VulkanPipeline;
	class VulkanShader;

	class RGDeferredCompositeNode final : public RenderGraphNode
	{
	public:
		RGDeferredCompositeNode(VulkanContext& context, RenderGraph* pRenderGraph);
		~RGDeferredCompositeNode();
	private:
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Draw(DrawInfo& context) override;
	private:
		VulkanContext& mContext;

		LoadStoreClearOp  mColorRtLoadStoreOp;

		// draw skybox
		RefPtr<VulkanPipelineSignature> mpSkyboxPipelineSignature;
		VulkanPipeline*                 mpSkyboxPipeline;
		RefPtr<VulkanShader>            mpSkyboxShader;

		// draw gui pipeline
		RefPtr<VulkanPipelineSignature> mpGBufferCompositePipelineSignature;
		VulkanPipeline* mpGBufferCompositePipeline;
		RefPtr<VulkanShader> mpGBufferCompositeShader;
	};

}
#endif // SG_ENABLE_DEFERRED_SHADING