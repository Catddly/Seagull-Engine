#pragma once

#include "Defs/Defs.h"

#include "RenderGraphResource.h"

#include <eastl/list.h>

namespace SG
{

	class RenderGraphNode
	{
	public:
		RenderGraphNode() = default;
		virtual ~RenderGraphNode() = default;

		void AttachResource(const char* name);
		void DetachResource(const char* name);
	protected:
		virtual void Prepare() = 0;
		virtual void Execute() = 0;
	protected:
		eastl::list<const char*> mAttachResources;
	private:
		friend class RenderGraph;
		friend class RenderGraphBuilder;
		RenderGraphNode* mpPrev;
		RenderGraphNode* mpNext;
	};

}