#pragma once

#include "Defs/Defs.h"

#include "RenderGraphResource.h"

#include "Stl/vector.h"
#include "eastl/optional.h"

namespace SG
{

#define SG_MAX_RENDER_GRAPH_NODE_RESOURCE 10

	class VulkanCommandBuffer;
	class VulkanRenderPass;

	class RenderGraphNode
	{
	public:
		RenderGraphNode();
		virtual ~RenderGraphNode() = default;
	protected:
		void AttachResource(UInt32 slot, const RenderGraphInReousrce& resource);
		void DetachResource(UInt32 slot);
		void DetachResource(const RenderGraphInReousrce& resource);

		void ClearResources();
	protected:
		virtual void Reset() = 0;
		virtual void Prepare(VulkanRenderPass* pRenderpass) = 0;
		virtual void Update(UInt32 frameIndex) = 0;
		virtual void Execute(VulkanCommandBuffer& pBuf) = 0;
	private:
		bool HaveValidResource() const;
	private:
		typedef eastl::optional<RenderGraphInReousrce> InResourceType;

		friend class RenderGraph;
		friend class RenderGraphBuilder;

		vector<InResourceType> mInResources;
		UInt32 mResourceValidFlag;
	};

}