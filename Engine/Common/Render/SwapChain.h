#pragma once

#include "Common/Base/BasicTypes.h"

#include "Common/Render/Renderer.h"

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

	struct Texture;
	struct SwapChain
	{
		virtual ~SwapChain() = default;

		virtual Texture*   GetTexture(UInt32 index) const = 0;
		virtual Handle     GetNativeHandle() const = 0;

		virtual EImageFormat GetColorFormat() const = 0;
	};

}