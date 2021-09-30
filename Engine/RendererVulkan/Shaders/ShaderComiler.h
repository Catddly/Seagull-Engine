#pragma once

#include "Render/Shader.h"

#include "Stl/string.h"

namespace SG
{

	class ShaderCompiler
	{
	public:
		//! Load SPIRV shaders in as ShaderStages.
		//! The format of shaders' name should be ***-vert.spv or ***-frag.spv for SPIRV shaders.
		//! @param [binShaderName] Just name of the shaders, like basic. it will try find all the basic-vert.spv, basic-frag.spv, etc...
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		static bool LoadSPIRVShader(const string& binShaderName, ShaderStages& outStages);
	};

}