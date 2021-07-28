#pragma once

#include "Common/Render/Shader.h"

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
		Renderer* mpRenderer = nullptr;

		EShaderLanguage mShaderLanguage;
		std::byte*      mBinary = nullptr;

		VkShaderModule  mShaderModule;
	};

}