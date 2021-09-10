#pragma once

#include "Base/BasicTypes.h"

namespace SG
{

	enum class EQueueType
	{
		eNull,
		eGraphic,
		eTransfer,
		eCompute,
		MAX_COUNT,
	};

	enum class EQueuePriority
	{
		eNormal,
		eHigh,
		eImmediate,
		MAX_COUNT,
	};

}