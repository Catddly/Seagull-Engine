#include "StdAfx.h"
#include "System/System.h"
#include "System/Module.h"

#include "Memory/Memory.h"

#include "Stl/string.h"

namespace SG
{

	eastl::map<string, IModule*> ModuleManager::mUserModuleMap;

	ModuleManager::ModuleManager()
	{

	}

	ModuleManager::~ModuleManager()
	{
		for (auto beg = mUserModuleMap.rbegin(); beg != mUserModuleMap.rend(); beg++)
		{
			beg->second->OnShutdown();
			Delete(beg->second);
		}
	}

	bool ModuleManager::RegisterUserModule(const char* name, IModule* pModule)
	{
		if (pModule)
		{
			pModule->OnInit();
			mUserModuleMap[name] = pModule;
			return true;
		}
		else
			return false;
	}

	IModule* ModuleManager::UnRegisterUserModule(const char* name)
	{
		auto iter = mUserModuleMap.find(name);
		if (iter != mUserModuleMap.end())
		{
			auto* pModule = mUserModuleMap[name];
			pModule->OnShutdown();
			mUserModuleMap.erase(iter);
			return pModule;
		}
		return nullptr;
	}

	void ModuleManager::Update(float deltaTime)
	{
		for (auto e : mUserModuleMap)
			e.second->OnUpdate(deltaTime);
	}

	void ModuleManager::Draw()
	{
		for (auto e : mUserModuleMap)
			e.second->OnDraw();
	}

}