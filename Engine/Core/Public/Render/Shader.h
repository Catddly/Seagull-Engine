#pragma once

#include "Base/BasicTypes.h"

#include <cstddef>
#include <eastl/map.h>

namespace SG
{

	enum class EShaderLanguage
	{
		eGLSL = 0,
		eHLSL,
		eMetal,
	};

	enum class EShaderStages : UInt32
	{
		efVert = 1 << 0,
		efTesc = 1 << 1,
		efTese = 1 << 2,
		efGeom = 1 << 3,
		efFrag = 1 << 4,
		efComp = 1 << 5,
		NUM_STAGES = 6,
	};
	SG_ENUM_CLASS_FLAG(UInt32, EShaderStages);

	//! Data of shader.
	struct ShaderData
	{
		std::byte*    pBinary = nullptr;
		Size          binarySize;
	};

	typedef eastl::map<EShaderStages, ShaderData> Shader;

}