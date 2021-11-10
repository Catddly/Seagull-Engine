#pragma once

#include "Base/BasicTypes.h"
#include "Defs/Defs.h"

#include "Math/Vector.h"

namespace SG
{

	struct ClearValue
	{
		Vector4f color = { 0.0f, 0.0f, 0.0f, 1.0f };
		struct
		{
			float   depth   = 1.0f;
			UInt32  stencil = 0;
		} depthStencil;
	};

	interface RenderContext
	{
		virtual ~RenderContext() = default;
	};

}