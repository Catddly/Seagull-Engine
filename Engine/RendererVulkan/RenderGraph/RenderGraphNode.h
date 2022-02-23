#pragma once

#include "Defs/Defs.h"

#include "RenderGraphResource.h"

#include "Stl/vector.h"

namespace SG
{

#define SG_MAX_RENDER_GRAPH_NODE_RESOURCE 10

	class VulkanCommandBuffer;
	class VulkanRenderPass;

	class RenderGraphNode
	{
	public:
		RenderGraphNode() : mpPrev(nullptr), mpNext(nullptr) {}
		virtual ~RenderGraphNode() = default;
	protected:
		void AttachResource(RenderGraphInReousrce resource);
		bool ReplaceResource(RenderGraphInReousrce oldResource, RenderGraphInReousrce newResource);
		void ReplaceOrAttachResource(RenderGraphInReousrce oldResource, RenderGraphInReousrce newResource);

		void DetachResource(RenderGraphInReousrce resource);
		void ClearResources();
	protected:
		virtual void Reset() = 0;
		virtual void Prepare(VulkanRenderPass* pRenderpass) = 0;
		virtual void Update(UInt32 frameIndex) = 0;
		virtual void Execute(VulkanCommandBuffer& pBuf) = 0;
	private:
		friend class RenderGraph;
		friend class RenderGraphBuilder;

		RenderGraphNode* mpPrev;
		RenderGraphNode* mpNext;

		vector<RenderGraphInReousrce> mInResources;
	};

}