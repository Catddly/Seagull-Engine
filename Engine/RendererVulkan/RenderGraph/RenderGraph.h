#pragma once

#include "Stl/vector.h"

#include "RenderGraphNode.h"

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
		explicit RenderGraph(const char* name, VulkanContext* pContext) : mName(name), mpRootNode(nullptr), mpRenderContext(pContext) {}
		~RenderGraph();

		void Draw(UInt32 frameIndex) const;
		void WindowResize();

		SG_INLINE const char* GetName() const { return mName; }
	private:
		//! Every RenderGraphNode call Prepare() to initialize the necessary data. 
		void Build();
		//! Clear all the temporary resources in this render graph.
		void Clear();
	private:
		friend class RenderGraphBuilder;
		VulkanContext*    mpRenderContext;
		VulkanRenderPass* mpCurrRenderPass;
		vector<VulkanFrameBuffer*> mpFrameBuffers;

		const char*      mName;
		RenderGraphNode* mpRootNode;
	};

}