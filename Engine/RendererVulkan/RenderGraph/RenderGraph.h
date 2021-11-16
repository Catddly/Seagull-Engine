#pragma once

#include "Stl/vector.h"

#include "RenderGraphNode.h"

namespace SG
{

	class RenderGraphBuilder
	{
	public:
		RenderGraphBuilder(RenderGraph& renderGraph) : mRenderGraph(renderGraph) {}
		~RenderGraphBuilder() = default;

		void NewRenderPass(RenderGraphNode* pNode);
	private:
		RenderGraph& mRenderGraph;
	};

	class RenderGraph final
	{
	public:
		explicit RenderGraph(const char* name) : mName(name), mpRootNode(nullptr) {}
		~RenderGraph() = default;

		SG_INLINE const char* GetName() const { return mName; }
	private:
		friend class RenderGraphBuilder;
		const char*      mName;
		RenderGraphNode* mpRootNode;
	};

}