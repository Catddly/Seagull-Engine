#include "StdAfx.h"
#include "VkConvert.h"

#include "System/ILogger.h"

namespace SG
{

	VkFormat ToVkImageFormat(EImageFormat format)
	{
		switch (format)
		{
		case EImageFormat::eNull: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUnorm_R1: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUnorm_R2: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUnorm_R4: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUnorm_R4G4: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUnorm_G4R4: return VK_FORMAT_R4G4_UNORM_PACK8;
		case EImageFormat::eUnorm_A8: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUnorm_R8: return VK_FORMAT_R8_UNORM;
		case EImageFormat::eUnorm_R8G8: return VK_FORMAT_R8G8_UNORM;
		case EImageFormat::eUnorm_G8R8: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUnorm_R8G8B8: return VK_FORMAT_R8G8B8_UNORM;
		case EImageFormat::eUnorm_R8G8B8A8: return VK_FORMAT_R8G8B8A8_UNORM;
		case EImageFormat::eSnorm_R8: return VK_FORMAT_R8_SNORM;
		case EImageFormat::eSnorm_R8G8: return VK_FORMAT_R8G8_SNORM;
		case EImageFormat::eSnorm_G8R8: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eSnorm_R8G8B8: return VK_FORMAT_R8G8B8_SNORM;
		case EImageFormat::eSnorm_R8G8B8A8: return VK_FORMAT_R8G8B8A8_SNORM;
		case EImageFormat::eUint_R8: return VK_FORMAT_R8_UINT;
		case EImageFormat::eUint_R8G8: return VK_FORMAT_R8G8_UINT;
		case EImageFormat::eUint_G8R8: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUint_R8G8B8: return VK_FORMAT_R8G8B8_UINT;
		case EImageFormat::eUint_R8G8B8A8: return VK_FORMAT_R8G8B8A8_UINT;
		case EImageFormat::eSint_R8: return VK_FORMAT_R8_SINT;
		case EImageFormat::eSint_R8G8: return VK_FORMAT_R8G8_SINT;
		case EImageFormat::eSint_R8G8B8: return VK_FORMAT_R8G8B8_SINT;
		case EImageFormat::eSint_G8R8: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eSint_R8G8B8A8: return VK_FORMAT_R8G8B8A8_SINT;
		case EImageFormat::eSrgb_R8: return VK_FORMAT_R8_SRGB;
		case EImageFormat::eSrgb_R8G8: return VK_FORMAT_R8G8_SRGB;
		case EImageFormat::eSrgb_G8R8: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eSrgb_R8G8B8: return VK_FORMAT_R8G8B8_SRGB;
		case EImageFormat::eSrgb_R8G8B8A8: return VK_FORMAT_R8G8B8A8_SRGB;
		case EImageFormat::eSrgb_B8G8R8A8: return VK_FORMAT_B8G8R8A8_SRGB;
		case EImageFormat::eUnorm_R16G16: return VK_FORMAT_R16G16_UNORM;
		case EImageFormat::eUnorm_G16R16: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eSnorm_R16G16: return VK_FORMAT_R16G16_SNORM;
		case EImageFormat::eSnorm_G16R16: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUint_R16G16: return VK_FORMAT_R16G16_UINT;
		case EImageFormat::eSint_R16G16: return VK_FORMAT_R16G16_SINT;
		case EImageFormat::eSfloat_R16G16: return VK_FORMAT_R16G16_SFLOAT;
		case EImageFormat::eSbfloat_R16G16: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eUnorm_A2R10G10B10: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		case EImageFormat::eUint_A2R10G10B10: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
		case EImageFormat::eSnorm_A2R10G10B10: return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
		case EImageFormat::eSint_A2R10G10B10: return VK_FORMAT_A2R10G10B10_SINT_PACK32;
		case EImageFormat::eUint_R32G32: return VK_FORMAT_R32G32_UINT;
		case EImageFormat::eSint_R32G32: return VK_FORMAT_R32G32_SINT;
		case EImageFormat::eSfloat_R32G32: return VK_FORMAT_R32G32_SFLOAT;
		case EImageFormat::eUint_R32G32B32: return VK_FORMAT_R32G32B32_UINT;
		case EImageFormat::eSint_R32G32B32: return VK_FORMAT_R32G32B32_SINT;
		case EImageFormat::eSfloat_R32G32B32: return VK_FORMAT_R32G32B32_SFLOAT;
		case EImageFormat::eUint_R32G32B32A32: return VK_FORMAT_R32G32B32A32_UINT;
		case EImageFormat::eSint_R32G32B32A32: return VK_FORMAT_R32G32B32A32_SINT;
		case EImageFormat::eSfloat_R32G32B32A32: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case EImageFormat::eUint_R64: return VK_FORMAT_R64_UINT;
		case EImageFormat::eSint_R64: return VK_FORMAT_R64_SINT;
		case EImageFormat::eSfloat_R64: return VK_FORMAT_R64_SFLOAT;
		case EImageFormat::eUint_R64G64: return VK_FORMAT_R64G64_UINT;
		case EImageFormat::eSint_R64G64: return VK_FORMAT_R64G64_SINT;
		case EImageFormat::eSfloat_R64G64: return VK_FORMAT_R64G64_SFLOAT;
		case EImageFormat::eUint_R64G64B64: return VK_FORMAT_R64G64B64_UINT;
		case EImageFormat::eSint_R64G64B64: return VK_FORMAT_R64G64B64_SINT;
		case EImageFormat::eSfloat_R64G64B64: return VK_FORMAT_R64G64B64_SFLOAT;
		case EImageFormat::eUint_R64G64B64A64: return VK_FORMAT_R64G64B64A64_UINT;
		case EImageFormat::eSint_R64G64B64A64: return VK_FORMAT_R64G64B64A64_SINT;
		case EImageFormat::eSfloat_R64G64B64A64: return VK_FORMAT_R64G64B64A64_SFLOAT;
		case EImageFormat::eUnorm_D16: return VK_FORMAT_D16_UNORM;
		case EImageFormat::eSfloat_D32: return VK_FORMAT_D32_SFLOAT;
		case EImageFormat::eUnorm_D16_uint_S8: return VK_FORMAT_D16_UNORM_S8_UINT;
		case EImageFormat::eUnorm_D24_uint_S8: return VK_FORMAT_D24_UNORM_S8_UINT;
		case EImageFormat::eSfloat_D32_uint_S8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case EImageFormat::eUnorm_B2G3R3: return VK_FORMAT_UNDEFINED;
		case EImageFormat::MAX_COUNT: return VK_FORMAT_UNDEFINED;
		}
		return VK_FORMAT_UNDEFINED;
	}

	SG::EImageFormat ToSGImageFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_UNDEFINED:
		case VK_FORMAT_R4G4_UNORM_PACK8:
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
		case VK_FORMAT_B5G6R5_UNORM_PACK16:
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return EImageFormat::eNull; break;

		case VK_FORMAT_R8_UNORM: return EImageFormat::eUnorm_R8; break;
		case VK_FORMAT_R8_SNORM: return EImageFormat::eSnorm_R8; break;
		case VK_FORMAT_R8_USCALED: return EImageFormat::eUnorm_R8; break;
		case VK_FORMAT_R8_SSCALED: return EImageFormat::eUnorm_R8; break;
		case VK_FORMAT_R8_UINT: return EImageFormat::eUint_R8; break;
		case VK_FORMAT_R8_SINT: return EImageFormat::eSint_R8; break;
		case VK_FORMAT_R8_SRGB: return EImageFormat::eSrgb_R8; break;

		case VK_FORMAT_R8G8_UNORM: return EImageFormat::eUnorm_R8G8; break;
		case VK_FORMAT_R8G8_SNORM: return EImageFormat::eSnorm_R8G8; break;
		case VK_FORMAT_R8G8_USCALED: return EImageFormat::eUnorm_R8G8; break;
		case VK_FORMAT_R8G8_SSCALED: return EImageFormat::eSnorm_R8G8; break;
		case VK_FORMAT_R8G8_UINT: return EImageFormat::eUint_R8G8; break;
		case VK_FORMAT_R8G8_SINT: return EImageFormat::eSint_R8G8; break;
		case VK_FORMAT_R8G8_SRGB: return EImageFormat::eSrgb_R8G8; break;
			
		case VK_FORMAT_R8G8B8_UNORM: return EImageFormat::eUnorm_R8G8B8; break;
		case VK_FORMAT_R8G8B8_SNORM: return EImageFormat::eSnorm_R8G8B8; break;
		case VK_FORMAT_R8G8B8_USCALED: return EImageFormat::eUnorm_R8G8B8; break;
		case VK_FORMAT_R8G8B8_SSCALED: return EImageFormat::eSnorm_R8G8B8; break;
		case VK_FORMAT_R8G8B8_UINT: return EImageFormat::eUint_R8G8B8; break;
		case VK_FORMAT_R8G8B8_SINT: return EImageFormat::eSint_R8G8B8; break;
		case VK_FORMAT_R8G8B8_SRGB: return EImageFormat::eSrgb_R8G8B8; break;

		case VK_FORMAT_B8G8R8_UNORM: return EImageFormat::eUnorm_B8G8R8; break;
		case VK_FORMAT_B8G8R8_SNORM: return EImageFormat::eSnorm_B8G8R8; break;
		case VK_FORMAT_B8G8R8_USCALED: return EImageFormat::eUnorm_B8G8R8; break;
		case VK_FORMAT_B8G8R8_SSCALED: return EImageFormat::eSnorm_B8G8R8; break;
		case VK_FORMAT_B8G8R8_UINT: return EImageFormat::eUint_B8G8R8; break;
		case VK_FORMAT_B8G8R8_SINT: return EImageFormat::eSint_B8G8R8; break;
		case VK_FORMAT_B8G8R8_SRGB: return EImageFormat::eSrgb_B8G8R8; break;
			
		case VK_FORMAT_R8G8B8A8_UNORM: return EImageFormat::eUnorm_R8G8B8A8; break;
		case VK_FORMAT_R8G8B8A8_SNORM: return EImageFormat::eSnorm_R8G8B8A8; break;
		case VK_FORMAT_R8G8B8A8_USCALED: return EImageFormat::eUnorm_R8G8B8A8; break;
		case VK_FORMAT_R8G8B8A8_SSCALED: return EImageFormat::eSnorm_R8G8B8A8; break;
		case VK_FORMAT_R8G8B8A8_UINT: return EImageFormat::eUint_R8G8B8A8; break;
		case VK_FORMAT_R8G8B8A8_SINT: return EImageFormat::eSint_R8G8B8A8; break;
		case VK_FORMAT_R8G8B8A8_SRGB: return EImageFormat::eSrgb_R8G8B8A8; break;

		case VK_FORMAT_B8G8R8A8_UNORM: return EImageFormat::eUnorm_B8G8R8A8; break;
		case VK_FORMAT_B8G8R8A8_SNORM: return EImageFormat::eSnorm_B8G8R8A8; break;
		case VK_FORMAT_B8G8R8A8_USCALED: return EImageFormat::eUnorm_B8G8R8A8; break;
		case VK_FORMAT_B8G8R8A8_SSCALED: return EImageFormat::eSnorm_B8G8R8A8; break;
		case VK_FORMAT_B8G8R8A8_UINT: return EImageFormat::eUint_B8G8R8A8; break;
		case VK_FORMAT_B8G8R8A8_SINT: return EImageFormat::eSint_B8G8R8A8; break;
		case VK_FORMAT_B8G8R8A8_SRGB: return EImageFormat::eSrgb_B8G8R8A8; break;
			
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return EImageFormat::eNull; break;

		case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return EImageFormat::eUnorm_A2R10G10B10; break;
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return EImageFormat::eSnorm_A2R10G10B10; break;
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return EImageFormat::eUnorm_A2R10G10B10; break;
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return EImageFormat::eSnorm_A2R10G10B10; break;
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:	return EImageFormat::eUint_A2R10G10B10; break;
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:	return EImageFormat::eSint_A2R10G10B10; break;

		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		case VK_FORMAT_A2B10G10R10_SINT_PACK32: return EImageFormat::eNull; break;
			
		case VK_FORMAT_R16_UNORM: return EImageFormat::eUnorm_R16; break;
		case VK_FORMAT_R16_SNORM: return EImageFormat::eSnorm_R16; break;
		case VK_FORMAT_R16_USCALED: return EImageFormat::eUnorm_R16; break;
		case VK_FORMAT_R16_SSCALED:	return EImageFormat::eSnorm_R16; break;
		case VK_FORMAT_R16_UINT: return EImageFormat::eUint_R16; break;
		case VK_FORMAT_R16_SINT: return EImageFormat::eSint_R16; break;
		case VK_FORMAT_R16_SFLOAT: return EImageFormat::eSfloat_R16; break;
			
		case VK_FORMAT_R16G16_UNORM: return EImageFormat::eUnorm_R16G16; break;
		case VK_FORMAT_R16G16_SNORM: return EImageFormat::eSnorm_R16G16; break;
		case VK_FORMAT_R16G16_USCALED: return EImageFormat::eUnorm_R16G16; break;
		case VK_FORMAT_R16G16_SSCALED: return EImageFormat::eSnorm_R16G16; break;
		case VK_FORMAT_R16G16_UINT: return EImageFormat::eUint_R16G16; break;
		case VK_FORMAT_R16G16_SINT: return EImageFormat::eSint_R16G16; break;
		case VK_FORMAT_R16G16_SFLOAT: return EImageFormat::eSfloat_R16G16; break;
			
		case VK_FORMAT_R16G16B16_UNORM: return EImageFormat::eUnorm_R16G16B16; break;
		case VK_FORMAT_R16G16B16_SNORM: return EImageFormat::eSnorm_R16G16B16; break;
		case VK_FORMAT_R16G16B16_USCALED: return EImageFormat::eUnorm_R16G16B16; break;
		case VK_FORMAT_R16G16B16_SSCALED: return EImageFormat::eSnorm_R16G16B16; break;
		case VK_FORMAT_R16G16B16_UINT: return EImageFormat::eUint_R16G16B16; break;
		case VK_FORMAT_R16G16B16_SINT: return EImageFormat::eSint_R16G16B16; break;
		case VK_FORMAT_R16G16B16_SFLOAT: return EImageFormat::eSfloat_R16G16B16; break;
			
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:	
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R16G16B16A16_SFLOAT: return EImageFormat::eNull; break;
			
		case VK_FORMAT_R32_UINT: return EImageFormat::eUint_R32G32; break;
		case VK_FORMAT_R32_SINT: return EImageFormat::eSint_R32G32; break;
		case VK_FORMAT_R32_SFLOAT: return EImageFormat::eSfloat_R32G32; break;
			
		case VK_FORMAT_R32G32_UINT: return EImageFormat::eUint_R32G32; break;
		case VK_FORMAT_R32G32_SINT: return EImageFormat::eSint_R32G32; break;
		case VK_FORMAT_R32G32_SFLOAT: return EImageFormat::eSfloat_R32G32; break;
			
		case VK_FORMAT_R32G32B32_UINT: return EImageFormat::eUint_R32G32B32; break;
		case VK_FORMAT_R32G32B32_SINT: return EImageFormat::eSint_R32G32B32; break;
		case VK_FORMAT_R32G32B32_SFLOAT: return EImageFormat::eSfloat_R32G32B32; break;

		case VK_FORMAT_R32G32B32A32_UINT: return EImageFormat::eUint_R32G32B32A32; break;
		case VK_FORMAT_R32G32B32A32_SINT: return EImageFormat::eSint_R32G32B32A32; break;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return EImageFormat::eSfloat_R32G32B32A32; break;
			
		case VK_FORMAT_R64_UINT: return EImageFormat::eUint_R64; break;
		case VK_FORMAT_R64_SINT: return EImageFormat::eSint_R64; break;
		case VK_FORMAT_R64_SFLOAT: return EImageFormat::eSfloat_R64; break;
			
		case VK_FORMAT_R64G64_UINT: return EImageFormat::eUint_R64G64; break;
		case VK_FORMAT_R64G64_SINT: return EImageFormat::eSint_R64G64; break;
		case VK_FORMAT_R64G64_SFLOAT: return EImageFormat::eSfloat_R64G64; break;
			
		case VK_FORMAT_R64G64B64_UINT: return EImageFormat::eUint_R64G64B64; break;
		case VK_FORMAT_R64G64B64_SINT: return EImageFormat::eSint_R64G64B64; break;
		case VK_FORMAT_R64G64B64_SFLOAT: return EImageFormat::eSfloat_R64G64B64; break;
			
		case VK_FORMAT_R64G64B64A64_UINT: return EImageFormat::eUint_R64G64B64A64; break;
		case VK_FORMAT_R64G64B64A64_SINT: return EImageFormat::eSint_R64G64B64A64; break;
		case VK_FORMAT_R64G64B64A64_SFLOAT: return EImageFormat::eSfloat_R64G64B64A64; break;
			
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return EImageFormat::eNull; break;
		case VK_FORMAT_D16_UNORM: return EImageFormat::eUnorm_D16; break;
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_S8_UINT: return EImageFormat::eNull; break;
		case VK_FORMAT_D16_UNORM_S8_UINT: return EImageFormat::eUnorm_D16_uint_S8; break;
		case VK_FORMAT_D24_UNORM_S8_UINT: return EImageFormat::eUnorm_D24_uint_S8; break;
		case VK_FORMAT_D32_SFLOAT_S8_UINT: return EImageFormat::eNull; break;
			
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		case VK_FORMAT_BC2_UNORM_BLOCK:
		case VK_FORMAT_BC2_SRGB_BLOCK:	
		case VK_FORMAT_BC3_UNORM_BLOCK:			
		case VK_FORMAT_BC3_SRGB_BLOCK:			
		case VK_FORMAT_BC4_UNORM_BLOCK:			
		case VK_FORMAT_BC4_SNORM_BLOCK:			
		case VK_FORMAT_BC5_UNORM_BLOCK:			
		case VK_FORMAT_BC5_SNORM_BLOCK:			
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:			
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
		case VK_FORMAT_BC7_UNORM_BLOCK:		
		case VK_FORMAT_BC7_SRGB_BLOCK: return EImageFormat::eNull; break;		
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:			
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:			
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:			
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:			
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return EImageFormat::eNull; break;
		case VK_FORMAT_EAC_R11_UNORM_BLOCK:	
		case VK_FORMAT_EAC_R11_SNORM_BLOCK:			
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:			
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:			
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:		
		case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:		
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:			
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:		
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:			
		case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:			
		case VK_FORMAT_G8B8G8R8_422_UNORM:			
		case VK_FORMAT_B8G8R8G8_422_UNORM:			
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:			
		case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:			
		case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:			
		case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:			
		case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:			
		case VK_FORMAT_R10X6_UNORM_PACK16:			
		case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:			
		case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:			
		case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:			
		case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:			
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:			
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:			
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:			
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:			
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:			
		case VK_FORMAT_R12X4_UNORM_PACK16:			
		case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:			
		case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:			
		case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:			
		case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:			
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:			
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:			
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:			
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:			
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:			
		case VK_FORMAT_G16B16G16R16_422_UNORM:			
		case VK_FORMAT_B16G16R16G16_422_UNORM:			
		case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:			
		case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:		
		case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:			
		case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:			
		case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:		
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:			
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:			
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:			
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:			
		case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:			
		case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:			
		case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:			
		case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:			
		case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:			
		case VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT:
		case VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT: return EImageFormat::eNull; break;
		default: SG_LOG_ERROR("Unknown vulkan image format!"); break;
		}
		return EImageFormat::eNull;
	}

	VkImageType ToVkImageType(EImageType type)
	{
		switch (type)
		{
		case SG::EImageType::e1D: return VK_IMAGE_TYPE_1D; break;
		case SG::EImageType::e2D: return VK_IMAGE_TYPE_2D; break;
		case SG::EImageType::e3D: return VK_IMAGE_TYPE_3D; break;
		default: SG_LOG_ERROR("Wrong Image type!"); break;
		}
		return VK_IMAGE_TYPE_MAX_ENUM;
	}

	SG::EImageType ToSGImageType(VkImageType type)
	{
		switch (type)
		{
		case VK_IMAGE_TYPE_1D: return EImageType::e1D; break;
		case VK_IMAGE_TYPE_2D: return EImageType::e2D; break;
		case VK_IMAGE_TYPE_3D: return EImageType::e3D; break;
		case VK_IMAGE_TYPE_MAX_ENUM:
		default: SG_LOG_ERROR("Unknown vulkan image type!"); break;
		}
		return EImageType::e2D;
	}

	VkSampleCountFlagBits ToVkSampleCount(ESampleCount cnt)
	{
		switch (cnt)
		{
		case SG::ESampleCount::eSample_1:  return VK_SAMPLE_COUNT_1_BIT; break;
		case SG::ESampleCount::eSample_2:  return VK_SAMPLE_COUNT_2_BIT; break;
		case SG::ESampleCount::eSample_4:  return VK_SAMPLE_COUNT_4_BIT; break;
		case SG::ESampleCount::eSample_8:  return VK_SAMPLE_COUNT_8_BIT; break;
		case SG::ESampleCount::eSample_16: return VK_SAMPLE_COUNT_16_BIT; break;
		case SG::ESampleCount::eSample_32: return VK_SAMPLE_COUNT_32_BIT; break;
		case SG::ESampleCount::eSample_64: return VK_SAMPLE_COUNT_64_BIT; break;
		default: SG_LOG_ERROR("Wrong sample count!"); break;
		}
		return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
	}

	SG::ESampleCount ToSGSampleCount(VkSampleCountFlagBits cnt)
	{
		switch (cnt)
		{
		case VK_SAMPLE_COUNT_1_BIT: return ESampleCount::eSample_1; break;
		case VK_SAMPLE_COUNT_2_BIT: return ESampleCount::eSample_2; break;
		case VK_SAMPLE_COUNT_4_BIT:	return ESampleCount::eSample_4; break;
		case VK_SAMPLE_COUNT_8_BIT:	return ESampleCount::eSample_8; break;
		case VK_SAMPLE_COUNT_16_BIT: return ESampleCount::eSample_16; break;
		case VK_SAMPLE_COUNT_32_BIT: return ESampleCount::eSample_32; break;
		case VK_SAMPLE_COUNT_64_BIT: return ESampleCount::eSample_64; break;
		case VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM:
		default: SG_LOG_ERROR("Unknown vulkan sample count!"); break;
		}
		return ESampleCount::eSample_1;
	}

	VkImageUsageFlags ToVkImageUsage(ERenderTargetUsage usage)
	{
		switch (usage)
		{
		case SG::ERenderTargetUsage::efTransfer_Src:                return VK_IMAGE_USAGE_TRANSFER_SRC_BIT; break;
		case SG::ERenderTargetUsage::efTransfer_Dst:                return VK_IMAGE_USAGE_TRANSFER_DST_BIT; break;
		case SG::ERenderTargetUsage::efSampled:                     return VK_IMAGE_USAGE_SAMPLED_BIT; break;
		case SG::ERenderTargetUsage::efStorage:                     return VK_IMAGE_USAGE_STORAGE_BIT; break;
		case SG::ERenderTargetUsage::efColor:                       return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; break;
		case SG::ERenderTargetUsage::efDepth_Stencil:               return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; break;
		case SG::ERenderTargetUsage::efTransient:                   return VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT; break;
		case SG::ERenderTargetUsage::efInput:                       return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; break;
		case SG::ERenderTargetUsage::efShading_Rate_Image:          return VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV; break;
		case SG::ERenderTargetUsage::efFragment_Density_Map:        return VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT; break;
		case SG::ERenderTargetUsage::efFragment_Shading_Rate_Image: return VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR; break;
		default: SG_LOG_ERROR("Wrong render target usage!"); break;
		}
		return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	SG::ERenderTargetUsage ToSGImageUsage(VkImageUsageFlags usage)
	{
		switch (usage)
		{
		case VK_IMAGE_USAGE_TRANSFER_SRC_BIT: return ERenderTargetUsage::efTransfer_Src; break;
		case VK_IMAGE_USAGE_TRANSFER_DST_BIT: return ERenderTargetUsage::efTransfer_Src; break;
		case VK_IMAGE_USAGE_SAMPLED_BIT: return ERenderTargetUsage::efSampled; break;
		case VK_IMAGE_USAGE_STORAGE_BIT: return ERenderTargetUsage::efStorage; break;
		case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: return ERenderTargetUsage::efColor; break;
		case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT: return ERenderTargetUsage::efDepth_Stencil; break;
		case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT: return ERenderTargetUsage::efTransient; break;
		case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT: return ERenderTargetUsage::efInput; break;
		case VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV: return ERenderTargetUsage::efShading_Rate_Image; break;
		case VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT: return ERenderTargetUsage::efFragment_Density_Map; break;
		case VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM:
		default:
			SG_LOG_ERROR("Unknown vulkan image usage!"); break;
		}
		return ERenderTargetUsage::efColor;
	}

	VkPresentModeKHR ToVkPresentMode(EPresentMode pmode)
	{
		switch (pmode)
		{
		case SG::EPresentMode::eImmediate:    return VK_PRESENT_MODE_IMMEDIATE_KHR;
		case SG::EPresentMode::eFIFO:         return VK_PRESENT_MODE_FIFO_KHR;
		case SG::EPresentMode::eFIFO_Relaxed: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		case SG::EPresentMode::eMailbox:      return VK_PRESENT_MODE_MAILBOX_KHR;
		case SG::EPresentMode::MAX_COUNT:
		default:
			SG_LOG_ERROR("Unsupported present mode");
			return VK_PRESENT_MODE_MAX_ENUM_KHR;
		}
	}

	VkAccessFlags ToVkAccessFlags(EResoureceBarrier barrier)
	{
		VkAccessFlags ret = 0;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efCopy_Source, barrier))
			ret |= VK_ACCESS_TRANSFER_READ_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efCopy_Dest, barrier))
			ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efVertex_And_Constant_Buffer, barrier))
			ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efIndexBuffer, barrier))
			ret |= VK_ACCESS_INDEX_READ_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efUnordered_Access, barrier))
			ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efIndirect_Argument, barrier))
			ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efRenderTarget, barrier))
			ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efDepth_Write, barrier))
			ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efShader_Resource, barrier))
			ret |= VK_ACCESS_SHADER_READ_BIT;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efPresent, barrier))
			ret |= VK_ACCESS_MEMORY_READ_BIT;
		return ret;
	}

	VkImageLayout ToVkImageLayout(EResoureceBarrier barrier)
	{
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efCopy_Source, barrier))
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efCopy_Dest, barrier))
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efRenderTarget, barrier))
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efDepth_Write, barrier))
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efUnordered_Access, barrier))
			return VK_IMAGE_LAYOUT_GENERAL;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efShader_Resource, barrier))
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if (SG_HAS_ENUM_FLAG(EResoureceBarrier::efPresent, barrier))
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		if (barrier == EResoureceBarrier::efCommon)
			return VK_IMAGE_LAYOUT_GENERAL;
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	VkImageViewType ToVkImageViewType(VkImageType imageType, UInt32 array)
	{
		SG_ASSERT(array >= 1);

		if (array == 1)
		{
			switch (imageType)
			{
			case VK_IMAGE_TYPE_1D: return VK_IMAGE_VIEW_TYPE_1D; break;
			case VK_IMAGE_TYPE_2D: return VK_IMAGE_VIEW_TYPE_2D; break;
			case VK_IMAGE_TYPE_3D: return VK_IMAGE_VIEW_TYPE_3D; break;
			default: SG_LOG_ERROR("Unacceptable image type!"); break;
			}
		}
		else
		{
			if (array == 6 && imageType == VK_IMAGE_TYPE_2D)
				return VK_IMAGE_VIEW_TYPE_CUBE;
			if (imageType == VK_IMAGE_TYPE_1D)
				return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			if (imageType == VK_IMAGE_TYPE_2D)
				return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

		}

		return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	}

	VkPipelineStageFlags ToVkPipelineStageFlags(VkAccessFlags accessFlags, EQueueType queueType)
	{
		VkPipelineStageFlags flags = 0;
		if (queueType == EQueueType::eGraphic)
		{
			if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

			if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
			{
				flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
				flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				//flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
				//flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
				//flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			}

			if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
				flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		}

		// Compatible with both compute and graphics queues
		if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
			flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

		if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
			flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

		if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
			flags |= VK_PIPELINE_STAGE_HOST_BIT;

		if (flags == 0)
			flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		return flags;
	}

}