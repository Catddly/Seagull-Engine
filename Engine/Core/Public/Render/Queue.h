#pragma once

#include "Base/BasicTypes.h"
#include "Defs/Defs.h"

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

	//interface Queue
	//{
	//	virtual ~Queue() = default;

	//	virtual bool SubmitCommands(RenderContext* pContext, UInt32 bufferIndex, RenderSemaphore* renderSemaphore, RenderSemaphore* presentSemaphore, RenderFence* fence) = 0;
	//	virtual void WaitIdle() const = 0;
	//};

}