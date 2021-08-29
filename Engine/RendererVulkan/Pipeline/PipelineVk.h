#pragma once

#include "Render/Pipeline.h"

namespace SG
{

	struct Renderer;
	struct Shader;

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
		PipelineVk(Renderer* pRenderer, Shader* pShader, EPipelineType type);
		~PipelineVk();

	private:
		Renderer*   mpRenderer = nullptr;
		Shader*     mpShader = nullptr;
		RenderPass* mpRenderpass = nullptr;

		EPipelineType mType;

		VkPipelineLayout mPipelineLayout;
		VkPipeline       mPipeline;
	};

}