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