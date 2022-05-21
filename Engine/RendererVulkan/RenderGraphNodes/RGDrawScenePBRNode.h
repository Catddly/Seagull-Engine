#pragma once

#include "Render/FrameBuffer.h"
#include "Event/MessageBus/MessageBus.h"

#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanContext;
	class VulkanShader;
	class VulkanPipeline;
	class VulkanPipelineSignature;

	class RGDrawScenePBRNode final : public RenderGraphNode
	{
	public:
		RGDrawScenePBRNode(VulkanContext& context, RenderGraph* pRenderGraph);
		~RGDrawScenePBRNode();
	private:
		virtual void Update() override;
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Draw(DrawInfo& context) override;
	private:
		void CreateColorRt();
		void DestroyColorRt();

		void GenerateBRDFLut();
		//! For IBL-PBR(image based lighting in physical based rendering) diffuse calculation.
		//! By using Split Sum Approximation.
		void PreCalcIrradianceCubemap();
		void PrefilterCubemap();
		void DrawScene(DrawInfo& drawInfo);

		void OnRenderDataRebuild();
	private:
		VulkanContext&        mContext;
		MessageBusMember      mMessageBusMember;

		LoadStoreClearOp      mColorRtLoadStoreOp;
		LoadStoreClearOp      mDepthRtLoadStoreOp;

		RefPtr<VulkanPipelineSignature> mpSkyboxPipelineSignature;
		VulkanPipeline*                 mpSkyboxPipeline;
		RefPtr<VulkanShader>            mpSkyboxShader;

		RefPtr<VulkanPipelineSignature> mpPipelineSignature;
		VulkanPipeline*					mpInstancePipeline;
		RefPtr<VulkanShader>            mpInstanceShader;
		VulkanPipeline*                 mpPipeline;
		RefPtr<VulkanShader>            mpShader;
	};

}