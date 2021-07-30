#pragma once

#include "Common/Base/BasicTypes.h"
#include "Common/Render/Renderer.h"

#include <cstddef>
#include <EASTL/map.h>

namespace SG
{

	enum class EShaderLanguage
	{
		eGLSL = 0,
		eHLSL,
		eMetal,
		eSpirv,
	};

	enum class EShaderStages : UInt32
	{
		eVert = 1 << 0,
		eTesc = 1 << 1,
		eTese = 1 << 2,
		eGeom = 1 << 3,
		eFrag = 1 << 4,
		eComp = 1 << 5,
	};
	SG_ENUM_CLASS_FLAG(UInt32, EShaderStages);

	//! Data of shader.
	struct ShaderData
	{
		std::byte* pBinary = nullptr;
		Size       binarySize;
		Handle     pShader; // shader handle specified by graphic api
	};

	typedef eastl::map<EShaderStages, ShaderData> ShaderStages;

	struct Shader
	{
		virtual ~Shader() = default;

		virtual ShaderStages* GetShaderStages() = 0;
	};

}