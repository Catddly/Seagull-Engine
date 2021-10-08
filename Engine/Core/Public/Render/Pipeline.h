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

	interface Pipeline
	{
		virtual ~Pipeline() = default;
	};

}