#include "StdAfx.h"
#include "Common/System/ISystem.h"

namespace SG
{

	eastl::map<const char*, IModule*> CModuleManager::mCoreModuleMap;
	eastl::map<const char*, IModule*> CModuleManager::mUserModuleMap;

	CModuleManager::CModuleManager()
	{

	}

	CModuleManager::~CModuleManager()
	{
		for (auto beg = mUserModuleMap.rbegin(); beg != mUserModuleMap.rend(); beg++)
		{
			beg->second->OnShutdown();
			delete beg->second;
		}

		for (auto beg = mCoreModuleMap.rbegin(); beg != mCoreModuleMap.rend(); beg++)
		{
			beg->second->OnShutdown();
			delete beg->second;
		}
	}

	bool CModuleManager::RegisterCoreModule(IModule* pModule)
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

	bool CModuleManager::RegisterUserModule(IModule* pModule)
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

	void CModuleManager::OnUpdate()
	{
		for (auto e : mUserModuleMap)
			e.second->OnUpdate();
		for (auto e : mCoreModuleMap)
			e.second->OnUpdate();
	}

}