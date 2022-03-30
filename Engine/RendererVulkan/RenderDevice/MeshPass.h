#pragma once

#include "Base/BasicTypes.h"

namespace SG
{

	enum class EMeshPass : UInt32
	{
		eForward = 0,
		eForwardInstanced,
		NUM_MESH_PASS,
		eUndefined,
	};

}