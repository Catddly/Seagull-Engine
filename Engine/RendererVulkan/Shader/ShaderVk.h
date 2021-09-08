#pragma once

#include "Render/Shader.h"

#include "Stl/string.h"
#include "Stl/string_view.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct Renderer;
	class ShaderVk : public Old_Shader
	{
	public:
		ShaderVk(Renderer* pRenderer, const string* shaderStages, Size numShaderStages);
		~ShaderVk();

		virtual ShaderStages* GetShaderStages() override;
	private:
		//! Compile shader by using glslc.exe from vulkanSDK.
		//! @param (name) the name of the shader.
		//! @param (extension) the extension of the shader.
		//! @return the full name of compiled shader (spirv).
		string CompileFromVulkanSDK(const string& name, const string& extension) const;
		//! Read the compiled shader (spirv) from disk.
		//! @param (filepath) where to get the binary.
		//! @return true if the binary is exist otherwise false.
		bool ReadBinaryFromDisk(const string& name, const string& extension);
		void CreateVulkanShaderModule(ShaderData* pShaderData);
	private:
		Renderer* mpRenderer = nullptr;

		EShaderLanguage mShaderLanguage;
		ShaderStages    mShaderStages;
	};

}