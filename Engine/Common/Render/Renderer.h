#pragma once

#include "Common/Config.h"

namespace SG
{

	struct Renderer
	{
		virtual ~Renderer() = default;

		SG_COMMON_API virtual bool OnInit() = 0;
		SG_COMMON_API virtual void OnShutdown() = 0;
	};

}