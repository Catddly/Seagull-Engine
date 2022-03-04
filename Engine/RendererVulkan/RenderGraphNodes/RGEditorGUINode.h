#pragma once

#include "Render/Shader.h"
#include "Render/FrameBuffer.h"
#include "Render/GUI/IGUIDriver.h"

// should we include this here??
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanContext;
	class VulkanRenderPass;

	class VulkanPipelineSignature;
	class VulkanPipeline;
	class VulkanShader;

	class RGEditorGUINode final : public RenderGraphNode
	{
	public:
		RGEditorGUINode(VulkanContext& context);
		~RGEditorGUINode();
	private:
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Update(float deltaTime, UInt32 frameIndex) override;
		virtual void Draw(RGDrawContext& context) override;
	private:
		VulkanContext&    mContext;
		VulkanRenderPass* mpRenderPass;

		LoadStoreClearOp  mColorRtLoadStoreOp;

		RefPtr<VulkanPipelineSignature> mpGUIPipelineSignature;
		VulkanPipeline* mpGUIPipeline;
		RefPtr<VulkanShader> mpGUIShader;

		UInt32 mCurrVertexCount;
		UInt32 mCurrIndexCount;
	};

}