#include "StdAfx.h"
#include "Render/SwapChain.h"

namespace SG
{

	UInt32 TextureIDAllocator::mCurrId = 1;

	UInt32 TextureIDAllocator::NewID()
	{
		return mCurrId++;
	}

}