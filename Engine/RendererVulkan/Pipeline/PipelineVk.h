#pragma once

#include "Common/Render/Pipeline.h"

namespace SG
{

	struct Renderer;
	struct Shader;
	class PipelineVk : public Pipeline
	{
	public:
		PipelineVk(Renderer* pRenderer, Shader* pShader, EPipelineType type);
		~PipelineVk();

	private:
		Renderer* mpRenderer = nullptr;
		Shader*   mpShader = nullptr;

		EPipelineType mType;
	};

}