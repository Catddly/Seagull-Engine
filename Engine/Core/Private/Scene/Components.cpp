#include "StdAfx.h"
#include "Scene/Components.h"

namespace SG
{

	static UInt32 gCurrObjectID = 0;

	UInt32 NewObjectID()
	{
		return gCurrObjectID++;
	}

}