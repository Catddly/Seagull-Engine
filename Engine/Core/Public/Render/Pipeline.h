#pragma once

#include "Render/Renderer.h"

namespace SG
{

	enum class EPipelineType
	{
		eGraphic = 0,
		eCompute,
		eTransfer,
		MAX_COUNT,
	};

	struct RenderPass
	{
		virtual ~RenderPass() = default;

		virtual Handle GetNativeHandle() const = 0;
	};

	struct Pipeline
	{
		virtual ~Pipeline() = default;
	};

}