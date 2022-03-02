#pragma once

#include "Render/Shader.h"
#include "Render/FrameBuffer.h"
#include "Render/GUI/IGUIDriver.h"

#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

namespace SG
{

	class VulkanContext;
	class VulkanRenderPass;

	class VulkanDescriptorSetLayout;
	class VulkanPipelineLayout;
	class VulkanPipeline;

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

		VulkanDescriptorSetLayout* mpGUITextureSetLayout;
		VulkanPipelineLayout*      mpGUIPipelineLayout;
		VulkanPipeline*            mpGUIPipeline;
		Shader                     mGUIShader;

		UInt32 mCurrVertexCount;
		UInt32 mCurrIndexCount;
	};

}