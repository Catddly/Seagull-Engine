#pragma once

#include "Render/FrameBuffer.h"
#include "Render/GUI/IGUIDriver.h"
#include "Event/MessageBus/MessageBus.h"

// should we include this here??
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanContext;
	class VulkanRenderPass;

	class VulkanPipelineSignature;
	class VulkanPipeline;
	class VulkanShader;

	class RGFinalOutputNode final : public RenderGraphNode
	{
	public:
		RGFinalOutputNode(VulkanContext& context, RenderGraph* pRenderGraph);
		~RGFinalOutputNode();
	private:
		virtual void Reset() override;
		virtual void Update() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Draw(RGDrawInfo& context) override;
	private:
		void GUIDraw(VulkanCommandBuffer& pBuf, UInt32 frameIndex);

		void OnEditorViewportResize(Vector2f& data);
	private:
		VulkanContext&    mContext;
		MessageBusMember  mMessageBusMember;

		LoadStoreClearOp  mColorRtLoadStoreOp;

		// draw composition pipeline
		RefPtr<VulkanPipelineSignature> mpCompPipelineSignature;
		VulkanPipeline* mpCompPipeline;
		RefPtr<VulkanShader> mpCompShader;

		// draw gui pipeline
		RefPtr<VulkanPipelineSignature> mpGUIPipelineSignature;
		VulkanPipeline* mpGUIPipeline;
		RefPtr<VulkanShader> mpGUIShader;

		UInt32 mCurrVertexCount;
		UInt32 mCurrIndexCount;
	};

}