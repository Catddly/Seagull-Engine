#include "StdAfx.h"
#include "Render/Shader/Shader.h"

namespace SG
{

	void ShaderAttributesLayout::CalculateLayoutOffsets()
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