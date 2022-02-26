#pragma once

#include "Render/Shader.h"
#include "Render/FrameBuffer.h"
#include "Render/GUI/GUIDriver.h"

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
		virtual void Update(UInt32 frameIndex) override;
		virtual void Execute(VulkanCommandBuffer& pBuf) override;
	private:
		VulkanContext&    mContext;
		VulkanRenderPass* mpRenderPass;

		LoadStoreClearOp  mColorRtLoadStoreOp;

		VulkanDescriptorSetLayout* mpGUITextureSetLayout;
		VulkanPipelineLayout*      mpGUIPipelineLayout;
		VulkanPipeline*            mpGUIPipeline;
		Shader                     mGUIShader;

		IGUIDriver* mpGUIDriver;

		UInt32 mCurrVertexCount;
		UInt32 mCurrIndexCount;
	};

}