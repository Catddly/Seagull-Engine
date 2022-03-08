#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

namespace SG
{

	enum class EResourceBarrier : UInt32
	{
		efUndefined = 0,
		efVertex_And_Constant_Buffer = 0x1,
		efIndexBuffer = 0x2,
		efRenderTarget = 0x4,
		efUnordered_Access = 0x8,
		efDepth_Write = 0x10,
		efDepth_Read = 0x20,
		efNon_Pixel_Shader_Resource = 0x40,
		efPixel_Shader_Resource = 0x80,
		efShader_Resource = 0x40 | 0x80,
		efStrem_Out = 0x100,
		efIndirect_Argument = 0x200,
		efCopy_Dest = 0x400,
		efCopy_Source = 0x800,
		efGeneric_Read = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		efPresent = 0x1000,
		efCommon = 0x2000,
		efStencil_Read = 0x4000,
		efStencil_Write = 0x8000,
		efDepth = efDepth_Write | efDepth_Read,
		efDepth_Stencil = efDepth_Write | efDepth_Read | efStencil_Read,
		efDepth_Stencil_Read_Only = efDepth_Stencil | efStencil_Write,
	};
	SG_ENUM_CLASS_FLAG(UInt32, EResourceBarrier);

}