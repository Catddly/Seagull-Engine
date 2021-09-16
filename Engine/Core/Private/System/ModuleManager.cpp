#include "StdAfx.h"
#include "System/System.h"

#include "Memory/IMemory.h"

namespace SG
{

	eastl::map<const char*, IModule*> ModuleManager::mCoreModuleMap;
	eastl::map<const char*, IModule*> ModuleManager::mUserModuleMap;

	ModuleManager::ModuleManager()
	{

	}

	ModuleManager::~ModuleManager()
	{
		for (auto beg = mUserModuleMap.rbegin(); beg != mUserModuleMap.rend(); beg++)
		{
			beg->second->OnShutdown();
			Memory::Delete(beg->second);
		}

		for (auto beg = mCoreModuleMap.rbegin(); beg != mCoreModuleMap.rend(); beg++)
		{
			beg->second->OnShutdown();
			Memory::Delete(beg->second);
		}
	}

	bool ModuleManager::RegisterCoreModule(IModule* pModule)
	{
		if (pModule)
		{
			pModule->OnInit();
			mCoreModuleMap[pModule->GetRegisterName()] = pModule;
			return true;
		}
		else
			return false;
	}

	bool ModuleManager::RegisterUserModule(IModule* pModule)
	{
		if (pModule)
		{
			pModule->OnInit();
			mUserModuleMap[pModule->GetRegisterName()] = pModule;
			return true;
		}
		else
			return false;
	}

	void ModuleManager::Update()
	{
		for (auto e : mUserModuleMap)
			e.second->OnUpdate();
		for (auto e : mCoreModuleMap)
			e.second->OnUpdate();
	}

}