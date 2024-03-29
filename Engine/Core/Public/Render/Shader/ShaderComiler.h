#pragma once

#include "Core/Config.h"
#include "Render/Shader/Shader.h"

#include "Stl/string.h"
#include "Stl/SmartPtr.h"

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
		SG_CORE_API static bool LoadSPIRVShader(const string& binShaderName, RefPtr<Shader> pShader);

		//! Load SPIRV shaders in as ShaderStages.
		//! The format of shaders' name should be ***-vert.spv or ***-frag.spv for SPIRV shaders.
		//! @param [vertShaderName] Name of the vertex shader.
		//! @param [fragShaderName] Name of the fragment shader.
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		SG_CORE_API static bool LoadSPIRVShader(const string& vertShaderName, const string& fragShaderName, RefPtr<Shader> pShader);

		//! Compile GLSL shaders to SPIRV shaders and load them in
		//! The format of shaders' name should be ***.vert or ***.frag for GLSL shaders.
		//! @param [binShaderName] Just name of the shaders, like basic. it will try find all the basic.vert, basic.frag, etc...
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		SG_CORE_API static bool CompileGLSLShader(const string& binShaderName, RefPtr<Shader> pShader);

		//! Compile GLSL shaders to SPIRV shaders and load them in
		//! The format of shaders' name should be ***.vert or ***.frag for GLSL shaders.
		//! @param [vertShaderName] Name of the vertex shader.
		//! @param [fragShaderName] Name of the fragment shader.
		//! @param [outStages] Output data of shader stages.
		//! @return If it successfully load in the shaders.
		SG_CORE_API static bool CompileGLSLShader(const string& vertShaderName, const string& fragShaderName, RefPtr<Shader> pShader);
	private:
		static bool ReadInShaderData(const string& name, UInt32 stage, RefPtr<Shader> pShader, UInt8& checkFlag);
		static bool CompileShaderVkSDK(const string& actualName, const string& compiledName, const string& pOut);
		static bool CompileShaderVendor(const string& actualName, const string& compiledName, const string& pOut);
		static bool CheckCompileError(const string& actualName, const string& outputMessage);

		//! Use spirv-cross to reflect shader info from .spv(compiled shader).
		static bool ReflectSPIRV(RefPtr<Shader> pShader);
	};

}