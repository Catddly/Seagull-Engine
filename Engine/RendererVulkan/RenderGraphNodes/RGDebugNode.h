#pragma once

#include "RendererVulkan/RenderGraph/RenderGraphNode.h"
#include "Scene/Scene.h"
#include "Event/MessageBus/MessageBus.h"

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
		virtual void Update() override;
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Draw(DrawInfo& context) override;
	private:
		void OnSelectedEntityChanged(Scene::TreeNode* pTreeNode);
	private:
		VulkanContext& mContext;
		MessageBusMember mMessageBusMember;

		LoadStoreClearOp mColorRtLoadStoreOp;
		LoadStoreClearOp mDepthRtLoadStoreOp;

		Matrix4f mDebugObjectModelMat;

		Scene::TreeNode* mpSelectedEntityTreeNode = nullptr;

		RefPtr<VulkanPipelineSignature> mpDebugLinePipelineSignature;
		VulkanPipeline*                 mpDebugLinePipeline;
		RefPtr<VulkanShader>            mpDebugLineShader;
	};

}