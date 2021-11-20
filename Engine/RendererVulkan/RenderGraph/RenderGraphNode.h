#pragma once

#include "Defs/Defs.h"

#include "RenderGraphResource.h"

#include <eastl/list.h>

namespace SG
{

	class VulkanCommandBuffer;
	class VulkanRenderPass;

	class RenderGraphNode
	{
	public:
		RenderGraphNode() : mpPrev(nullptr), mpNext(nullptr) {}
		virtual ~RenderGraphNode() = default;

		//void AttachResource(const char* name);
		//void DetachResource(const char* name);
	protected:
		virtual VulkanRenderPass* Prepare() = 0;
		virtual void Execute(VulkanCommandBuffer& pBuf) = 0;
		virtual void Clear() = 0;
	protected:
		//eastl::list<const char*> mAttachResources;
	private:
		friend class RenderGraph;
		friend class RenderGraphBuilder;
		RenderGraphNode* mpPrev;
		RenderGraphNode* mpNext;
	};

}