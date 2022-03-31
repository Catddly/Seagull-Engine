#pragma once

#include "Defs/Defs.h"

namespace SG
{

	enum class EPipelineType
	{
		eGraphic = 0,
		eCompute,
		eTransfer,
		MAX_COUNT,
	};

	enum class EVertexInputRate
	{
		ePerVertex = 0,
		ePerInstance
	};

	enum class ECullMode
	{
		eBack = 0,
		eFront,
		eFrontAndBack,
		eNone,
	};

	enum class EPolygonMode
	{
		eFill = 0,
		eLine,
		ePoint,
	};

	interface Pipeline
	{
		virtual ~Pipeline() = default;
	};

}