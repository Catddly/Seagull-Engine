#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"
#include "Base/BasicTypes.h"

namespace SG
{

	interface IModule
	{
		virtual ~IModule() = default;
		using module_t = void;

		SG_CORE_API virtual void OnInit() = 0;
		SG_CORE_API virtual void OnShutdown() = 0;

		SG_CORE_API virtual void OnUpdate() {}

		SG_CORE_API virtual const char* GetRegisterName() const = 0;
	};

}