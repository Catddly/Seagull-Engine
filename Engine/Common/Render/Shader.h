#pragma once

namespace SG
{

	enum class EShaderLanguage
	{
		eGLSL = 0,
		eHLSL,
		eMetal,
		eSpirv,
	};

	struct Shader
	{
		virtual ~Shader() = default;
	};

}