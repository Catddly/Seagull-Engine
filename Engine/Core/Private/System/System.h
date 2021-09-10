#pragma once

#include "Core/Config.h"
#include "System/ISystem.h"
#include "Thread/IThread.h"

#include "System/ILogger.h"
#include "System/IFileSystem.h"
#include "Platform/IOperatingSystem.h"

#include "User/IApp.h"

#include "Stl/string.h"

namespace SG
{

	class CSystem final : public ISystem
	{
	public:
		~CSystem() = default;

		SG_CORE_API virtual void OnInit() override;
		SG_CORE_API virtual bool InitCoreModules() override;
		SG_CORE_API virtual void OnShutdown() override;

		//SG_CORE_API virtual SSystemModules* GetSystemModules() override;

		// TOOD: other modules should be loaded as dll,
		// don't use get/set function.
		//SG_CORE_API virtual void       SetI3DEngine(I3DEngine* p3DEngine) override;
		//SG_CORE_API virtual I3DEngine* GetI3DEngine() override;
		//SG_CORE_API virtual void       SetI2DEngine(I2DEngine* p3DEngine) override;
		//SG_CORE_API virtual I2DEngine* GetI2DEngine() override;

		template <class T>
		T GetModule(const char* name) const;

		template <class T>
		bool RegisterModule();

		SG_CORE_API virtual ILogger* GetLogger() const override;
		SG_CORE_API virtual IFileSystem* GetFileSystem() const override;
		SG_CORE_API virtual IInputSystem* GetInputSystem() const override;
		SG_CORE_API virtual IOperatingSystem* GetOS() const override;

		//! Check if all the core modules is loaded.
		SG_CORE_API virtual bool ValidateCoreModules() const override;
		//! Check if all the modules is loaded.
		SG_CORE_API virtual bool ValidateAllModules() const override;

		SG_CORE_API virtual bool SystemMainLoop() override;

		//! Add an IProcess to system to update.
		SG_CORE_API virtual void AddIProcess(IProcess* pProcess) override;
		//! Remove an IProcess from system.
		//virtual void RemoveIProcess(IProcess* pProcess) override;

		//! Get current memory usage for all the modules.
		SG_CORE_API virtual UInt32 GetTotalMemoryUsage() const override;
		//! Set engine's resource files root directory in absolute path.
		//! Default root directory will be the folder where .exe is in.
		//! \param (filepath) relative path to the .exe
		SG_CORE_API virtual void        SetRootDirectory(const char* filepath) override;
		SG_CORE_API virtual string      GetResourceDirectory(EResourceDirectory rd) const override;

		SG_CORE_API virtual int RunProcess(const char* pCommand, const char** ppArgs, Size argNum, const char* pOut) override;

		//! Force to use ISystemManager as the interface of system manager.
		SG_CORE_API static CSystem* GetInstance();
	protected:
		CSystem();
		friend struct Memory;
	private:
		void OnUpdate();
	private:
		enum {
			SG_MAX_FILE_PATH = 200,
			SG_MAX_DRIVE_PATH = 5,
			SG_MAX_FILE_NAME = 30,
			SG_MAX_EXT_PATH = 10,
			SG_MAX_DIREC_PATH = SG_MAX_FILE_PATH - SG_MAX_DRIVE_PATH - SG_MAX_FILE_NAME - SG_MAX_EXT_PATH,
		};

		static CSystem* sInstance;
		CModuleManager mModuleManager;

		IProcess* mpCurrActiveProcess;
		Thread  mMainThread;
		string  mRootPath;
	};

	template <class T>
	bool SG::CSystem::RegisterModule()
	{
		T* pModule = Memory::New<T>();
		if (pModule)
		{
			mModuleManager.RegisterUserModule(pModule);
			return true;
		}
		return false;
	}

	template <class T>
	T SG::CSystem::GetModule(const char* name) const
	{
		return mModuleManager.GetModule<T>(name);
	}

}