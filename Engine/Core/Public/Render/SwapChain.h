#pragma once

#include "Base/BasicTypes.h"

#include "Platform/IOperatingSystem.h"

namespace SG
{

	// TODO: move to the other place
	enum class EImageFormat
	{
		eNull = 0,
		eUnorm_R1,
		eUnorm_R2,
		eUnorm_R4,
		eUnorm_R4G4,
		eUnorm_G4R4,
		eUnorm_A8,
		eUnorm_R8,
		eUnorm_R8G8,
		eUnorm_G8R8,
		eUnorm_R8G8B8,
		eUnorm_R8G8B8A8,
		eSnorm_R8,
		eSnorm_R8G8,
		eSnorm_G8R8,
		eSnorm_R8G8B8,
		eSnorm_R8G8B8A8,
		eUint_R8,
		eUint_R8G8,
		eUint_G8R8,
		eUint_R8G8B8,
		eUint_R8G8B8A8,
		eSint_R8,
		eSint_R8G8,
		eSint_G8R8,
		eSint_R8G8DB8,
		eSint_R8G8DB8A8,
		eSrgb_R8,
		eSrgb_R8G8,
		eSrgb_G8R8,
		eSrgb_R8G8B8,
		eSrgb_R8G8B8A8,
		eSrgb_B8G8R8A8,
		eUnorm_R16G16,
		eUnorm_G16R16,
		eSnorm_R16G16,
		eSnorm_G16R16,
		eUint_R16G16,
		eSint_R16G16,
		eSfloat_R16G16,
		eSbfloat_R16G16,
		eUnorm_A2R10G10B10,
		eUint_A2R10G10B10,
		eSnorm_A2R10G10B10,
		eSint_A2R10G10B10,
		eUint_R32G32,
		eSint_R32G32,
		eSfloat_R32G32,
		eUint_R32G32B32,
		eSint_R32G32B32,
		eSfloat_R32G32B32,
		eUint_R32G32B32A32,
		eSint_R32G32B32A32,
		eSfloat_R32G32B32A32,
		eUint_R64,
		eSint_R64,
		eSfloat_R64,
		eUint_R64G64,
		eSint_R64G64,
		eSfloat_R64G64,
		eUint_R64G64B64,
		eSint_R64G64B64,
		eSfloat_R64G64B64,
		eUint_R64G64B64A64,
		eSint_R64G64B64A64,
		eSfloat_R64G64B64A64,
		eUnorm_D16,
		eSfloat_D32,
		eUnorm_D16_uint_S8,
		eUnorm_D24_uint_S8,
		eSfloat_D32_uint_S8,
		eUnorm_B2G3R3,
		MAX_COUNT,
	};

	enum class EPresentMode
	{
		eImmediate = 0,
		eFIFO,
		eFIFO_Relaxed,
		eMailbox,
		MAX_COUNT,
	};

	enum class ESampleCount
	{
		eSample_1 = 0,
		eSample_2,
		eSample_4,
		eSample_8,
		eSample_16,
		eSample_32,
		eSample_64,
	};

	enum class EImageType
	{
		e1D = 0,
		e2D,
		e3D,
	};

	enum class ERenderTargetUsage : UInt32
	{
		eTransfer_Src                = 1 << 0,
		eTransfer_Dst                = 1 << 1,
		eSampled                     = 1 << 2,
		eStorage                     = 1 << 3,
		eColor                       = 1 << 4,
		eDepth_Stencil               = 1 << 5,
		eTransient                   = 1 << 6,
		eInput                       = 1 << 7,
		eShading_Rate_Image          = 1 << 8,
		eFragment_Density_Map        = 1 << 9,
		eFragment_Shading_Rate_Image = 1 << 10,
	};
	SG_ENUM_CLASS_FLAG(UInt32, ERenderTargetUsage);

	struct RenderTargetCreateDesc
	{
		EImageFormat       format;
		ESampleCount       sample;
		EImageType         type;
		ERenderTargetUsage usage;

		UInt32         width;
		UInt32         height;
		UInt32         depth;
		UInt32         array;
	};

}