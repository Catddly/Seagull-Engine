#pragma once

#include "Defs/Defs.h"

#include "RenderGraphResource.h"

#include "Stl/vector.h"
#include "eastl/array.h"
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

		struct RGDrawInfo
		{
			VulkanCommandBuffer* pCmd;
			UInt32               frameIndex;
		};
	protected:
		void AttachResource(UInt32 slot, const RenderGraphInReousrce& resource);
		void DetachResource(UInt32 slot);
		void DetachResource(const RenderGraphInReousrce& resource);

		void ClearResources();
	protected:
		//! Be called when receive window resize event or other reset status.
		virtual void Reset() = 0;
		//! Be called when the render graph finish compiling, user can use renderpass to create pipeline.
		virtual void Prepare(VulkanRenderPass* pRenderpass) = 0;
		//! Be called every frame to update the necessary resource which used to render.
		virtual void Update(UInt32 frameIndex) = 0;
		//! Be called every frame to record render command.
		virtual void Draw(RGDrawInfo& context) = 0;
	private:
		bool HaveValidResource() const;
	private:
		typedef eastl::optional<RenderGraphInReousrce> InResourceType;

		friend class RenderGraph;
		friend class RenderGraphBuilder;

		eastl::array<InResourceType, SG_MAX_RENDER_GRAPH_NODE_RESOURCE> mInResources;
		UInt32 mResourceValidFlag;
	};

}