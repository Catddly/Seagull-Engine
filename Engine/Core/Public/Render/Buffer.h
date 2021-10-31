#pragma once

#include "Base/BasicTypes.h"
#include "Defs/Defs.h"

namespace SG
{

	enum class EBufferType : UInt32
	{
		efTransfer_Src = 1 << 0,
		efTransfer_Dst = 1 << 1,
		efTexel_Uniform = 1 << 2,
		efTexel_Storage = 1 << 3,
		efUniform = 1 << 4,
		efStorage = 1 << 5,
		efIndex = 1 << 6,
		efVertex = 1 << 7,
		efIndirect = 1 << 8,
	};
	SG_ENUM_CLASS_FLAG(UInt32, EBufferType);

	struct BufferCreateDesc
	{
		void*        pData;
		EBufferType  type;
		UInt32       totalSizeInByte;

		bool         bLocal : 1;
	};

}