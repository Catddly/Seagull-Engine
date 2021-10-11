#pragma once

#include "Core/Config.h"
#include "Render/Shader.h"

#include "Stl/string.h"

namespace SG
{

	class SG_CORE_API ShaderCompiler
	{
	public:
		//! Load SPIRV shaders in as ShaderStages.
		//! The format of shaders' name should be ***-vert.spv or ***-frag.spv for SPIRV shaders.
		//! @param [binShaderName] Just name of the shaders, like basic. it will try find all the basic-vert.spv, basic-frag.spv, etc...
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		static bool LoadSPIRVShader(const string& binShaderName, Shader& outStages);

		//! Compile GLSL shaders to SPIRV shaders and load them in
		//! The format of shaders' name should be ***.vert or ***.frag for GLSL shaders.
		//! @param [binShaderName] Just name of the shaders, like basic. it will try find all the basic.vert, basic.frag, etc...
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		static bool CompileGLSLShader(const string& binShaderName, Shader& outStages);
	};

}