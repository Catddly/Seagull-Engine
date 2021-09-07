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

	typedef void* Old_QueueHandle;

	struct Old_Queue
	{
		virtual ~Old_Queue() = default;

		virtual EQueueType GetQueueType() const = 0;
		virtual EQueuePriority GetPriority() const = 0;
		virtual bool       IsValid() const = 0;
		virtual UInt32     GetQueueIndex() const = 0; // maybe we don't want this

		virtual Old_QueueHandle  GetNativeHandle() = 0;
	};

}