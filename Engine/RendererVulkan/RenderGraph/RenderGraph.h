#pragma once

#include "Stl/vector.h"

#include "RenderGraphNode.h"

#include <eastl/hash_map.h>

namespace SG
{

	class VulkanFrameBuffer;
	class VulkanRenderTarget;

	class RenderGraphBuilder
	{
	public:
		RenderGraphBuilder(RenderGraph& renderGraph) : mRenderGraph(renderGraph) {}
		~RenderGraphBuilder() = default;

		RenderGraphBuilder& NewRenderPass(RenderGraphNode* pNode);
		void                Complete();
	private:
		RenderGraph& mRenderGraph;
	};

	class VulkanContext;

	class RenderGraph final
	{
	public:
		explicit RenderGraph(const char* name, VulkanContext* pContext);
		~RenderGraph();

		void Update();
		void Draw(UInt32 frameIndex) const;

		void WindowResize();

		SG_INLINE const char* GetName() const { return mName; }
	private:
		//! Compile the render graph to create necessary data for renderer to use.
		//! If the nodes of this render graph had changed, compile it again.
		void Compile();
	private:
		friend class RenderGraphBuilder;
		VulkanContext* mpRenderContext;
		mutable UInt32 mFrameIndex = 0;

		eastl::hash_map<Size, VulkanRenderPass*>  mRenderPassesMap;
		eastl::hash_map<Size, VulkanFrameBuffer*> mFrameBuffersMap;

		VulkanRenderPass* mpCurrRenderPass;

		const char*      mName;
		RenderGraphNode* mpRootNode;
	};

}