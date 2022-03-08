#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "Reflection/Name.h"

#include <EASTL/map.h>

namespace SG
{

	interface IModule;
	class ModuleManager
	{
	public:
		ModuleManager();
		~ModuleManager();

		SG_CORE_API static void Update(float deltaTime);
		SG_CORE_API static void Draw();

		SG_CORE_API static bool     RegisterUserModule(const char* name, IModule* pModule);
		SG_CORE_API static IModule* UnRegisterUserModule(const char* name);

		template <class T>
		static T GetModule();
	private:
		SG_CORE_API static eastl::map<const char*, IModule*> mUserModuleMap;
	};

	template <class T>
	T ModuleManager::GetModule()
	{
		IModule* pModule = nullptr;
		pModule = mUserModuleMap[Refl::CT_TypeName<T>()];
		return static_cast<T>(pModule);
	}

}