#pragma once

#include "Render/FrameBuffer.h"
#include "Scene/Camera/ICamera.h"

#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "volk.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanContext;

	class VulkanRenderTarget;
	class VulkanShader;
	class VulkanPipeline;
	class VulkanPipelineSignature;

	class RGDrawSceneNode final : public RenderGraphNode
	{
	public:
		RGDrawSceneNode(VulkanContext& context, RenderGraph* pRenderGraph);
		~RGDrawSceneNode();
	private:
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Draw(DrawInfo& context) override;
	private:
		void DrawScene(VulkanCommandBuffer& pBuf);
	private:
		VulkanContext&        mContext;
		LoadStoreClearOp      mColorRtLoadStoreOp;
		LoadStoreClearOp      mDepthRtLoadStoreOp;

		RefPtr<VulkanPipelineSignature> mpSkyboxPipelineSignature;
		VulkanPipeline*                 mpSkyboxPipeline;
		RefPtr<VulkanShader>            mpSkyboxShader;

		RefPtr<VulkanPipelineSignature> mpPipelineSignature;
		VulkanPipeline*                 mpPipeline;
		RefPtr<VulkanShader>            mpShader;
	};

}