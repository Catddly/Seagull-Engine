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
		eUnknown,
	};

	SG_INLINE static const char* MeshTypeToExtString(EMeshType type)
	{
		switch (type)
		{
		case EMeshType::eOBJ: return ".obj"; break;
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


}