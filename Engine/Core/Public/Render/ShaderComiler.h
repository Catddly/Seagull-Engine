#pragma once

#include "Core/Config.h"
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
		SG_CORE_API static bool LoadSPIRVShader(const string& binShaderName, Shader* pShader);

		//! Load SPIRV shaders in as ShaderStages.
		//! The format of shaders' name should be ***-vert.spv or ***-frag.spv for SPIRV shaders.
		//! @param [vertShaderName] Name of the vertex shader.
		//! @param [fragShaderName] Name of the fragment shader.
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		SG_CORE_API static bool LoadSPIRVShader(const string& vertShaderName, const string& fragShaderName, Shader* pShader);

		//! Compile GLSL shaders to SPIRV shaders and load them in
		//! The format of shaders' name should be ***.vert or ***.frag for GLSL shaders.
		//! @param [binShaderName] Just name of the shaders, like basic. it will try find all the basic.vert, basic.frag, etc...
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		SG_CORE_API static bool CompileGLSLShader(const string& binShaderName, Shader* pShader);

		//! Compile GLSL shaders to SPIRV shaders and load them in
		//! The format of shaders' name should be ***.vert or ***.frag for GLSL shaders.
		//! @param [vertShaderName] Name of the vertex shader.
		//! @param [fragShaderName] Name of the fragment shader.
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		SG_CORE_API static bool CompileGLSLShader(const string& vertShaderName, const string& fragShaderName, Shader* pShader);
	private:
		static void ReadInShaderData(const string& name, UInt32 stage, Shader* pShader, UInt8& checkFlag);
		static bool CompileShaderVkSDK(const string& actualName, const string& compiledName, string& exePath, const string& pOut);

		//! Use spirv-cross to reflect shader info from .spv(compied shader).
		static bool ReflectSPIRV(Shader* pShader);
	};

}