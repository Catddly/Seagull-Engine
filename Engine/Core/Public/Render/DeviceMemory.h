#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

namespace SG
{

	enum class EGPUMemoryUsage
	{
		eUnknown = 0,
		eGPU_Only,
		eCPU_Only,
		eCPU_To_GPU,
		eGPU_To_CPU,
		eCPU_Copy,
		eGPU_Lazily,
		eAuto,
		eAuto_Prefer_Device,
		eAuto_Prefer_Host,
	};

	static bool IsHostVisible(EGPUMemoryUsage usage)
	{
		return !((usage == EGPUMemoryUsage::eGPU_Only) ||
			(usage == EGPUMemoryUsage::eGPU_Lazily));
	}

	enum class EGPUMemoryFlag : UInt32
	{
		efInvalid = 0,
		efPersistent_Map = BIT(0),
		efDedicated_Memory = BIT(1), //! Used it for big resource, e.g. attachments and images.
	};
	SG_ENUM_CLASS_FLAG(UInt32, EGPUMemoryFlag);

}