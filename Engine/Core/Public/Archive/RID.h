#pragma once

#include "Base/BasicTypes.h"

namespace SG
{

	class RID
	{
	public:
		static const UInt32 ID_MASK          = 0x00FFffFF;
		static const UInt32 INITIALIZED_MASK = 0x80000000;

		UInt32 id;
	private:

	};

}