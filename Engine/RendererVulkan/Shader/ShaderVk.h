#pragma once

#include "Common/Render/Shader.h"

#include "Common/Stl/string.h"
#include "Common/Stl/string_view.h"
#include <cstddef>

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct Renderer;
	class ShaderVk : public Shader
	{
	public:
		ShaderVk(Renderer* pRenderer, string_view filepath);
		~ShaderVk();
	private:
		//! Compile shader by using glslc.exe from vulkanSDK.
		//! @param (name) the name of the shader.
		//! @param (extension) the extension of the shader.
		//! @return the full name of compiled shader (spirv).
		string CompileFromVulkanSDK(const string& name, const string& extension) const;
		//! Read the compiled shader (spirv) from disk.
		//! @param (filepath) where to get the binary.
		//! @return true if the binary is exist otherwise false.
		bool ReadBinaryFromDisk(const char* filepath);
	private:
		Renderer* mpRenderer = nullptr;

		EShaderLanguage mShaderLanguage;
		std::byte*      mBinary = nullptr;
		Size            mBinarySize;

		VkShaderModule  mShaderModule;
	};

}