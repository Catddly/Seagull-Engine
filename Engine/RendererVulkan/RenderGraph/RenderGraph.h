#pragma once

#include "Stl/vector.h"

#include "RenderGraphNode.h"
#include "RenderGraphDependency.h"

#include "Stl/SmartPtr.h"
#include <eastl/hash_map.h>

namespace SG
{

	class VulkanContext;
	class VulkanFrameBuffer;
	class VulkanRenderTarget;

	class RenderGraphBuilder
	{
	public:
		RenderGraphBuilder(const char* name, VulkanContext* pContext);
		~RenderGraphBuilder();

		RenderGraphBuilder& NewRenderPass(RenderGraphNode* pNode);
		UniquePtr<RenderGraph> Build();
	private:
		UniquePtr<RenderGraph> mpRenderGraph = nullptr;
		bool mbInitSuccess = false;
	};

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

		void CompileRenderPasses(const RenderGraphNode* pCurrNode);
		void CompileFrameBuffers(const RenderGraphNode* pCurrNode);
	private:
		friend class RenderGraphBuilder;
		VulkanContext* mpContext;
		mutable UInt32 mFrameIndex = 0;

		eastl::hash_map<Size, VulkanRenderPass*>  mRenderPassesMap;
		// Why vector here?
		// Because we see each render graph resource as one resource, it may contain multiple render target in it.
		// In Most Case, the size of the vector is always one.
		eastl::hash_map<Size, VulkanFrameBuffer*> mFrameBuffersMap;
		VulkanRenderPass* mpCurrRenderPass;

		const char* mName;
		vector<RenderGraphNode*> mpNodes;

		RGResourceStatusKeeper mResourceStatusKeeper;
	};

}