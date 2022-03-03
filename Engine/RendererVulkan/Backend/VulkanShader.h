#pragma once

#include "Render/Shader.h"

#include "volk.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanContext;

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(VulkanContext& context);
		~VulkanShader() = default;

		void CreatePipelineShader();
		void DestroyPipelineShader();
		const vector<VkPipelineShaderStageCreateInfo>& GetShaderStagesCI() const { return mShaderStagesCI; }
	private:
		VulkanContext& mContext;

		vector<VkPipelineShaderStageCreateInfo> mShaderStagesCI;
		vector<VkShaderModule>                  mShaderModules;
	};

}