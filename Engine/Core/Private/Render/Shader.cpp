#include "StdAfx.h"
#include "Render/Shader.h"

namespace SG
{

	void VertexLayout::Append(const element_t& e)
	{
		mLayouts.emplace_back(e);
		mLayouts.back().offset = mTotalSize;
		mTotalSize += mLayouts.back().size;
	}

	void VertexLayout::CalculateLayoutOffsets()
	{
		if (mLayouts.empty())
		{
			mTotalSize = 0;
			return;
		}

		UInt32 accumulate = mLayouts[0].size;
		for (UInt32 i = 1; i < mLayouts.size(); ++i)
		{
			mLayouts[i].offset = accumulate;
			accumulate += mLayouts[i].size;
		}

		mTotalSize = accumulate;
	}

}