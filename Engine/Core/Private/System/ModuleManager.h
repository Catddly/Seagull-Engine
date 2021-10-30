#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"
#include "Base/BasicTypes.h"

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

		SG_CORE_API static bool     RegisterUserModule(IModule* pModule);
		SG_CORE_API static IModule* UnRegisterUserModule(const char* name);

		template <class T>
		static T GetModule(const char* name);
	private:
		SG_CORE_API static eastl::map<const char*, IModule*> mUserModuleMap;
	};

	template <class T>
	T ModuleManager::GetModule(const char* name)
	{
		IModule* pModule = nullptr;
		pModule = mUserModuleMap[name];
		return static_cast<T>(pModule);
	}

}