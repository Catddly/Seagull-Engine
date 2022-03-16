#pragma once

#include "Render/Shader/Shader.h"

#include "volk.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanDevice;

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(VulkanDevice& context);
		~VulkanShader() = default;

		static RefPtr<VulkanShader> Create(VulkanDevice& context);
	private:
		friend class VulkanPipeline;
		void CreatePipelineShader();
		void DestroyPipelineShader();
		const vector<VkPipelineShaderStageCreateInfo>& GetShaderStagesCI() const { return mShaderStagesCI; }
	private:
		VulkanDevice& mDevice;
		// after the pipeline creation, these data all will be eliminated.
		vector<VkPipelineShaderStageCreateInfo> mShaderStagesCI;
		vector<VkShaderModule>                  mShaderModules;
	};

}