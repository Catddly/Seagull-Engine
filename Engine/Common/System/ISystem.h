#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

#include "Common/System/IFileSystem.h"

#include "Common/Stl/string.h"
#include <EASTL/map.h>

namespace SG
{

	struct IModule;
	class CModuleManager
	{
	public:
		CModuleManager();
		~CModuleManager();

		SG_COMMON_API static void OnUpdate();
		// In order to ensure the order of core modules' destructions, we separate two independent register function.
		SG_COMMON_API static bool RegisterCoreModule(IModule* pModule);
		SG_COMMON_API static bool RegisterUserModule(IModule* pModule);

		template <class T>
		static T GetModule(const char* name, bool bIsCoreModule = false);
	private:
		SG_COMMON_API static eastl::map<const char*, IModule*> mCoreModuleMap;
		SG_COMMON_API static eastl::map<const char*, IModule*> mUserModuleMap;
	};

	template <class T>
	T CModuleManager::GetModule(const char* name, bool bIsCoreModule)
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

	struct IProcess;

	struct ILog;
	struct IFileSystem;
	struct IInputSystem;
	struct IOperatingSystem;

	//! System Manager to manager all the modules' life cycle
	//! and usage.
	//! Core modules are ILog, IFileSystem.
	//! High level modules are I3DEngine, I2DEngine.
	struct ISystem
	{
		virtual ~ISystem() = default;

		virtual void OnInit() = 0;
		virtual void OnShutdown() = 0;

		virtual bool InitCoreModules() = 0;

		virtual ILog*             GetLogger() const = 0;
		virtual IFileSystem*      GetFileSystem() const = 0;
		virtual IInputSystem*     GetInputSystem() const = 0;
		virtual IOperatingSystem* GetOS() const = 0;

		//! Register a user application.
		//virtual void RegisterUserApp(IApp* pApp) = 0;

		//! Register a user module.
		virtual bool RegisterModule(IModule* pModule) = 0;
		
		//! System main game loop.
		//! @return true if the loop exits safely, otherwise it is false.
		virtual bool SystemMainLoop() = 0;

		//! Check if all the core modules is loaded.
		virtual bool ValidateCoreModules() const = 0;
		//! Check if all the modules is loaded.
		virtual bool ValidateAllModules() const = 0;

		//! Add an IProcess to system to update.
		virtual void AddIProcess(IProcess* pProcess) = 0;
		//! Remove an IProcess from system.
		//virtual void RemoveIProcess(IProcess* pProcess) = 0;

		//! Get current memory usage for all the modules.
		virtual UInt32 GetTotalMemoryUsage() const = 0;
		//! Set engine's resource files root directory in absolute path.
		//! Default root directory will be the folder where .exe is in.
		//! \param (filepath) relative path to the .exe
		virtual void        SetRootDirectory(const char* filepath) = 0;
		virtual string      GetResourceDirectory(EResourceDirectory rd) const = 0;

		virtual int         RunProcess(const char* pCommand, const char** ppArgs, Size argNum, const char* pOut) = 0;
	};

}