#pragma once

#include "Base/BasicTypes.h"

#include "Platform/OS.h"

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
		eUnorm_B8G8R8,
		eUnorm_R8G8B8A8,
		eUnorm_B8G8R8A8,
		eSnorm_R8,
		eSnorm_R8G8,
		eSnorm_G8R8,
		eSnorm_R8G8B8,
		eSnorm_B8G8R8,
		eSnorm_R8G8B8A8,
		eSnorm_B8G8R8A8,
		eUint_R8,
		eUint_R8G8,
		eUint_G8R8,
		eUint_R8G8B8,
		eUint_B8G8R8,
		eUint_R8G8B8A8,
		eUint_B8G8R8A8,
		eSint_R8,
		eSint_R8G8,
		eSint_G8R8,
		eSint_R8G8B8,
		eSint_B8G8R8,
		eSint_R8G8B8A8,
		eSint_B8G8R8A8,
		eSrgb_R8,
		eSrgb_R8G8,
		eSrgb_G8R8,
		eSrgb_R8G8B8,
		eSrgb_B8G8R8,
		eSrgb_R8G8B8A8,
		eSrgb_B8G8R8A8,
		eUnorm_R16,
		eUnorm_R16G16,
		eUnorm_G16R16,
		eUnorm_R16G16B16,
		eSnorm_R16,
		eSnorm_R16G16,
		eSnorm_G16R16,
		eSnorm_R16G16B16,
		eUint_R16G16,
		eUint_R16G16B16,
		eSint_R16G16,
		eSint_R16G16B16,
		eSfloat_R16,
		eSfloat_R16G16,
		eSfloat_R16G16B16,
		eSbfloat_R16G16,
		eUnorm_A2R10G10B10,
		eUint_A2R10G10B10,
		eSnorm_A2R10G10B10,
		eSint_A2R10G10B10,
		eUint_R16,
		eUint_R32G32,
		eSint_R16,
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

		eDDSKTX_C_BC1,      // DXT1
		eDDSKTX_C_BC2,      // DXT3
		eDDSKTX_C_BC3,      // DXT5
		eDDSKTX_C_BC4,      // ATI1
		eDDSKTX_C_BC5,      // ATI2
		eDDSKTX_C_BC6H,     // BC6H
		eDDSKTX_C_BC7,      // BC7
		eDDSKTX_C_ETC1,     // ETC1 RGB8
		eDDSKTX_C_ETC2,     // ETC2 RGB8
		eDDSKTX_C_ETC2A,    // ETC2 RGBA8
		eDDSKTX_C_ETC2A1,   // ETC2 RGBA8A1
		eDDSKTX_C_PTC12,    // PVRTC1 RGB 2bpp
		eDDSKTX_C_PTC14,    // PVRTC1 RGB 4bpp
		eDDSKTX_C_PTC12A,   // PVRTC1 RGBA 2bpp
		eDDSKTX_C_PTC14A,   // PVRTC1 RGBA 4bpp
		eDDSKTX_C_PTC22,    // PVRTC2 RGBA 2bpp
		eDDSKTX_C_PTC24,    // PVRTC2 RGBA 4bpp
		eDDSKTX_C_ATC,      // ATC RGB 4BPP
		eDDSKTX_C_ATCE,     // ATCE RGBA 8 BPP explicit alpha
		eDDSKTX_C_ATCI,     // ATCI RGBA 8 BPP interpolated alpha
		eDDSKTX_C_ASTC4x4,  // ASTC 4x4 8.0 BPP
		eDDSKTX_C_ASTC5x5,  // ASTC 5x5 5.12 BPP
		eDDSKTX_C_ASTC6x6,  // ASTC 6x6 3.56 BPP
		eDDSKTX_C_ASTC8x5,  // ASTC 8x5 3.20 BPP
		eDDSKTX_C_ASTC8x6,  // ASTC 8x6 2.67 BPP
		eDDSKTX_C_ASTC10x5, // ASTC 10x5 2.56 BPP
		eDDSKTX_A8,
		eDDSKTX_R8,
		eDDSKTX_RGBA8,
		eDDSKTX_RGBA8S,
		eDDSKTX_RG16,
		eDDSKTX_RGB8,
		eDDSKTX_R16,
		eDDSKTX_R32F,
		eDDSKTX_R16F,
		eDDSKTX_RG16F,
		eDDSKTX_RG16S,
		eDDSKTX_RGBA16F,
		eDDSKTX_RGBA16,
		eDDSKTX_BGRA8,
		eDDSKTX_RGB10A2,
		eDDSKTX_RG11B10F,
		eDDSKTX_RG8,
		eDDSKTX_RG8S,

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

	enum class EImageUsage : UInt32
	{
		efTransfer_Src                = 1 << 0,
		efTransfer_Dst                = 1 << 1,
		efSample                      = 1 << 2,
		efStorage                     = 1 << 3,
		efColor                       = 1 << 4,
		efDepth_Stencil               = 1 << 5,
		efTransient                   = 1 << 6,
		efInput                       = 1 << 7,
		efShading_Rate_Image          = 1 << 8,
		efFragment_Density_Map        = 1 << 9,
		efFragment_Shading_Rate_Image = 1 << 10,
	};
	SG_ENUM_CLASS_FLAG(UInt32, EImageUsage);

	enum class EImageState
	{
		eComplete = 0,
		eIncomplete,
		eFailure,
	};

	enum class EFilterMode
	{
		eLinear = 0,
		eNearest,
	};

	enum class EAddressMode
	{
		eRepeat = 0,
		eMirrored_Repeat,
		eClamp_To_Edge,
		eClamp_To_Border,
		eMirrored_Clamp_To_Edge,
	};

	struct SamplerCreateDesc
	{
		const char* name;
		EFilterMode  filterMode;
		EAddressMode addressMode;
		float minLod;
		float maxLod;
		float lodBias;
	};

	struct TextureCopyRegion
	{
		UInt32 width;
		UInt32 height;
		UInt32 depth;
		UInt32 offset;

		UInt32 mipLevel;
		UInt32 baseArray;
		UInt32 layer;
	};

	struct TextureCreateDesc
	{
		const char*    name;
		EImageFormat   format;
		ESampleCount   sample;
		EImageType     type;
		EImageUsage    usage;

		UInt32         width;
		UInt32         height;
		UInt32         depth;
		UInt32         array;
		UInt32         mipLevel;

		UInt32         sizeInByte;
		const void*    pInitData;
	};

	interface RenderTarget
	{
		virtual ~RenderTarget() = default;

		virtual UInt32 GetWidth()     const = 0;
		virtual UInt32 GetHeight()    const = 0;
		virtual UInt32 GetDepth()     const = 0;
		virtual UInt32 GetNumArray()  const = 0;
		virtual UInt32 GetNumMipmap() const = 0;

		virtual EImageFormat GetFormat() const = 0;
		virtual ESampleCount GetSample() const = 0;
		virtual EImageType   GetType()   const = 0;
		virtual EImageUsage  GetUsage()  const = 0;
	};

}