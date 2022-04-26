#pragma once

#include "Defs/Defs.h"

#include "RendererVulkan/Renderer/RenderInfo.h"
#include "RenderGraphResource.h"

#include "Stl/vector.h"
#include "eastl/array.h"
#include "eastl/optional.h"

namespace SG
{

#define SG_MAX_RENDER_GRAPH_NODE_RESOURCE 10

	class VulkanCommandBuffer;
	class VulkanRenderPass;
	class RenderGraph;

	class RenderGraphNode
	{
	public:
		typedef eastl::optional<RenderGraphInReousrce> InResourceType;

		RenderGraphNode(RenderGraph* pRenderGraph);
		virtual ~RenderGraphNode() = default;
	protected:
		const InResourceType& GetResource(UInt32 slot);
		void AttachResource(UInt32 slot, const RenderGraphInReousrce& resource);
		void DetachResource(UInt32 slot);
		void DetachResource(const RenderGraphInReousrce& resource);

		void ClearResources();

		//! Callback function to notify the render graph this node had changed the frame buffer.
		void ResetFrameBuffer(Size frameBufferHash);
	protected:
		//! Be called when node need to update its resources. (optional)
		virtual void Update() {};
		//! Be called when receive window resize event or other reset status.
		virtual void Reset() = 0;
		//! Be called when the render graph finish compiling, user can use renderpass to create pipeline.
		virtual void Prepare(VulkanRenderPass* pRenderpass) = 0;
		//! Be called every frame to record render command.
		virtual void Draw(DrawInfo& context) = 0;
	private:
		bool HaveValidResource() const;
	private:
		friend class RenderGraph;
		friend class RenderGraphBuilder;

		RenderGraph* mpRenderGraph = nullptr;
		eastl::array<InResourceType, SG_MAX_RENDER_GRAPH_NODE_RESOURCE> mInResources;
		UInt32 mResourceValidFlag;
	};

}