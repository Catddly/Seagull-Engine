#pragma once

#include "Defs/Defs.h"

namespace SG
{

	interface IGUIDriver
	{
	public:
		virtual ~IGUIDriver() = default;

		virtual bool OnInit() = 0;
		virtual void OnShutdown() = 0;

		virtual void OnDraw() = 0;
	};

}