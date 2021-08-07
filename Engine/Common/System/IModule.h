#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

namespace SG
{

	struct SG_COMMON_API IModule
	{
		virtual ~IModule() = default;

		virtual void OnInit() = 0;
		virtual void OnShutdown() = 0;

		virtual void OnUpdate() {}

		virtual const char* GetRegisterName() const = 0;
	};

}