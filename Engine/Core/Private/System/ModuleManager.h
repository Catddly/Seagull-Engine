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
		// In order to ensure the order of core modules' destructions, we separate two independent register function.
		SG_CORE_API static bool RegisterCoreModule(IModule* pModule);

		SG_CORE_API static bool RegisterUserModule(IModule* pModule);
		SG_CORE_API static IModule* UnRegisterUserModule(const char* name);

		template <class T>
		static T GetModule(const char* name, bool bIsCoreModule = false);
	private:
		SG_CORE_API static eastl::map<const char*, IModule*> mCoreModuleMap;
		SG_CORE_API static eastl::map<const char*, IModule*> mUserModuleMap;
	};

	template <class T>
	T ModuleManager::GetModule(const char* name, bool bIsCoreModule)
	{
		IModule* pModule = nullptr;
		if (bIsCoreModule)
		{
			pModule = mCoreModuleMap[name];
			return static_cast<T>(pModule);
		}
		else
		{
			pModule = mUserModuleMap[name];
			return static_cast<T>(pModule);
		}
		return nullptr;
	}

}