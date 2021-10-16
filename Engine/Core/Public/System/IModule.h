#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"
#include "Base/BasicTypes.h"

namespace SG
{

	interface SG_CORE_API IModule
	{
		virtual ~IModule() = default;
		using module_t = void;

		virtual void OnInit() = 0;
		virtual void OnShutdown() = 0;
		
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnDraw()   {}
		
		virtual const char* GetRegisterName() const = 0;
	};

}