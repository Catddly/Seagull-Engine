#include "StdAfx.h"
#include "VulkanConversion.h"

#include "Common/System/ILog.h"

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
		case EImageFormat::eSint_G8R8: return VK_FORMAT_UNDEFINED;
		case EImageFormat::eSint_R8G8DB8: return VK_FORMAT_R8G8B8_SINT;
		case EImageFormat::eSint_R8G8DB8A8: return VK_FORMAT_R8G8B8A8_SINT;
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
		return EImageFormat::eNull;
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

}