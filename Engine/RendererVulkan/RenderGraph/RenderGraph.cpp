#include "StdAfx.h"
#include "RenderGraph.h"

#include "System/Logger.h"

namespace SG
{

	void RenderGraphBuilder::NewRenderPass(RenderGraphNode* pNode)
	{
		if (!mRenderGraph.mpRootNode)
			mRenderGraph.mpRootNode = pNode;
		else
		{
			auto* pCurrNode = mRenderGraph.mpRootNode;
			while (true)
			{
				if (pCurrNode->mpPrev)
					pCurrNode = pCurrNode->mpPrev;
				else
					pCurrNode->mpPrev = pNode;
			}	
		}
	}

}