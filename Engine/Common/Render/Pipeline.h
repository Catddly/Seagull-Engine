#pragma once

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
	};

	struct Pipeline
	{
		virtual ~Pipeline() = default;
	};

}