#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

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
		const char*  name;
		EBufferType  type;
		UInt32       bufferSize = 0;

		const void*  pInitData;
		UInt32       dataSize = 0;
		UInt32       dataOffset = 0;

		bool         bLocal;
		bool         bSubBufer = false;
	};

}