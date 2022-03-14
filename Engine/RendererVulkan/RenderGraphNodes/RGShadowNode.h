#pragma once

#include "RendererVulkan/RenderGraph/CommonUBO.h"
#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "Stl/SmartPtr.h"
#include "Math/MathBasic.h"

namespace SG
{

	class VulkanContext;
	class VulkanPipelineSignature;
	class VulkanPipeline;
	class VulkanShader;
	class VulkanGeometry;
	class VulkanRenderTarget;

	class DirectionalLight;

	class RGShadowNode final : public RenderGraphNode
	{
	public:
		RGShadowNode(VulkanContext& context);
		~RGShadowNode();
	private:
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Update(UInt32 frameIndex) override;
		virtual void Draw(RGDrawContext& context) override;
	private:
		VulkanContext& mContext;

		RefPtr<VulkanShader> mpShadowShader;
		RefPtr<VulkanPipelineSignature> mpShadowPipelineSignature;
		VulkanPipeline* mpShadowPipeline;

		VulkanGeometry* mpModelGeometry;
		DirectionalLight* mpDirectionalLight = nullptr;

		ShadowUBO mShadowUBO;

		struct PushConstant
		{
			Matrix4f model;
		};
		PushConstant mPushConstantModel;

		LoadStoreClearOp    mDepthRtLoadStoreOp;
	};

}