#pragma once

#include "Render/Pipeline.h"

namespace SG
{

	struct Renderer;
	struct Old_Shader;

	class RenderPassVk : public RenderPass
	{
	public:
		RenderPassVk(Renderer* pRenderer);
		~RenderPassVk();

		virtual Handle GetNativeHandle() const override;
	private:
		Renderer* mpRenderer = nullptr;

		VkRenderPass mRenderPass;
	};

	class PipelineVk : public Pipeline
	{
	public:
		PipelineVk(Renderer* pRenderer, Old_Shader* pShader, EPipelineType type);
		~PipelineVk();

	private:
		Renderer*   mpRenderer = nullptr;
		Old_Shader*     mpShader = nullptr;
		RenderPass* mpRenderpass = nullptr;

		EPipelineType mType;

		VkPipelineLayout mPipelineLayout;
		VkPipeline       mPipeline;
	};

}