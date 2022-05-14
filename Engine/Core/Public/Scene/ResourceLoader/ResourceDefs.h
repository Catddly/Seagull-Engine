#pragma once

#include "Defs/Defs.h"
#include "System/Logger.h"

namespace SG
{

	enum class ETextureType
	{
		ePNG = 0,
		eJPG,
		eKTX,
		eDDS,
		eUnknown,
	};

	enum class EMeshType
	{
		eOBJ = 0,
		eGLTF,
		eUnknown,
	};

	SG_INLINE static const char* MeshTypeToExtString(EMeshType type)
	{
		switch (type)
		{
		case EMeshType::eOBJ: return ".obj"; break;
		case EMeshType::eGLTF: return ".gltf"; break;
		case EMeshType::eUnknown:
		default:
			SG_LOG_ERROR("Invalid mesh type!");
			break;
		}
		return ".invalid";
	}

	enum class EResourceTypeCategory
	{
		eTexture = 0,
		eMesh,
		eSaveData,
	};

	//! Transient raw 2D texture data, will automatically free its memory when unused.
	struct SG_CORE_API Raw2DTexture
	{
		UInt32 width;
		UInt32 height;
		UInt32 array;
		UInt32 mipLevel;
		UInt32 sizeInByte;
		UInt32 dimention;

		ETextureType   type;
		unsigned char* pData = nullptr;
		void* pUserData = nullptr;

		SG_INLINE bool IsValid() const noexcept
		{
			return pData != nullptr;
		}

		SG_INLINE void FreeMemory() noexcept
		{
			free(pData);
			pData = nullptr;
		}

		Raw2DTexture() = default;
		Raw2DTexture(UInt32 w, UInt32 h, UInt32 arr, UInt32 mip, UInt32 dim)
			: width(w), height(h), array(arr), mipLevel(mip), dimention(dim), type(ETextureType::eUnknown), pData(nullptr)
		{}

		~Raw2DTexture()
		{
			//! Warning!! this is dangerous. Figure out a better way to do this.
			if (pData && !pUserData)
				FreeMemory();
		}
	};

}