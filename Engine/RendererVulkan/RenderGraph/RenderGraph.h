#pragma once

#include "Stl/vector.h"

#include "RenderGraphNode.h"
#include "RenderGraphDependency.h"

#include "Stl/SmartPtr.h"
#include "eastl/type_traits.h"
#include "eastl/hash_map.h"

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

		template <typename TRenderPassNode>
		RenderGraphBuilder& NewRenderPass();
		UniquePtr<RenderGraph> Build();
	private:
		VulkanContext* mpContext;
		UniquePtr<RenderGraph> mpRenderGraph = nullptr;
		bool mbInitSuccess = false;
	};

	template <typename TRenderPassNode>
	RenderGraphBuilder& RenderGraphBuilder::NewRenderPass()
	{
		static_assert(eastl::is_base_of_v<RenderGraphNode, TRenderPassNode>);
		auto* pNode = New(TRenderPassNode, *mpContext, mpRenderGraph.get());
		mpRenderGraph->mpNodes.push_back(pNode);
		return *this;
	}

	class VulkanRenderTarget;

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
		void ResetFrameBuffer(RenderGraphNode* pNode, Size frameBufferHash) noexcept;

		void AddResourceDenpendency(VulkanRenderTarget* pRenderTarget, EResourceBarrier srcStatus, EResourceBarrier dstStatus);
		void RemoveResourceDenpendency(VulkanRenderTarget* pRenderTarget);

		//! Compile the render graph to create necessary data for renderer to use.
		//! If the nodes of this render graph had changed, compile it again.
		void Compile();

		VulkanRenderPass* CompileRenderPasses(const RenderGraphNode* pCurrNode);
		void CompileFrameBuffers(const RenderGraphNode* pCurrNode);
	private:
		friend class RenderGraphNode;
		friend class RenderGraphBuilder;
		const char* mName;
		VulkanContext* mpContext;

		//! Render pass describes how to draw at a certain stage of the GPU pipeline.
		eastl::hash_map<Size, VulkanRenderPass*>  mRenderPassesMap;
		//! Frame buffer describes what to draw at a certain stage of the GPU pipeline.
		eastl::hash_map<Size, VulkanFrameBuffer*> mFrameBuffersMap;

		vector<RenderGraphNode*> mpNodes;

		RGResourceStatusKeeper mResourceStatusKeeper;
	};

}