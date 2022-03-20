#pragma once

#include "Render/FrameBuffer.h"
#include "Scene/Camera/ICamera.h"
#include "Scene/Light/PointLight.h"

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

	class VulkanGeometry;

	class RGDrawScenePBRNode final : public RenderGraphNode
	{
	public:
		RGDrawScenePBRNode(VulkanContext& context);
		~RGDrawScenePBRNode();
	private:
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Update(UInt32 frameIndex) override;
		virtual void Draw(RGDrawContext& context) override;
	private:
		void GenerateBRDFLut();
		//! For IBL-PBR(image based lighting in physical based rendering) diffuse calculation.
		//! By using Split Sum Approximation.
		void PreCalcIrradianceCubemap();
		void PrefilterCubemap();
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

		VulkanGeometry*   mpModelGeometry;
		VulkanGeometry*   mpGridGeometry;
		VulkanGeometry*   mpSkyboxGeometry;
		const PointLight* mpPointLight;
		ICamera*          mpCamera;

		struct PushConstant
		{
			Matrix4f model;
			Matrix4f inverseTransposeModel;
		};
		PushConstant mPushConstantGeo;
		PushConstant mPushConstantGrid;
	};

}