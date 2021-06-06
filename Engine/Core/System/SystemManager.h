#pragma once

#include "Core/Config.h"
#include "Common/System/ISystem.h"

#include "Common/System/ILog.h"
#include "Common/System/I2DEngine.h"
#include "Common/System/I3DEngine.h"
#include "Common/System/IFileSystem.h"

#include "Common/User/IApp.h"

#include "Core/Stl/string.h"

namespace SG
{

	class CSystemManager final : public ISystemManager
	{
	public:
		CSystemManager();
		~CSystemManager() = default;

		SG_CORE_API virtual void InitSystemEnv() override;
		SG_CORE_API virtual bool InitCoreModules() override;
		SG_CORE_API virtual void Update() override; // do we really want this??
		SG_CORE_API virtual void Shutdown() override;

		SG_CORE_API virtual ISystemModules* GetSystemModules() override;

		// TOOD: other modules should be loaded as dll,
		// don't use get/set function.
		SG_CORE_API virtual void       SetI3DEngine(I3DEngine* p3DEngine) override;
		SG_CORE_API virtual I3DEngine* GetI3DEngine() override;
		SG_CORE_API virtual void       SetI2DEngine(I2DEngine* p3DEngine) override;
		SG_CORE_API virtual I2DEngine* GetI2DEngine() override;

		SG_CORE_API virtual ILog* GetILog() override;
		SG_CORE_API virtual IFileSystem* GetIFileSystem() override;

		//! Check if all the core modules is loaded.
		SG_CORE_API virtual bool ValidateCoreModules() const override;
		//! Check if all the modules is loaded.
		SG_CORE_API virtual bool ValidateAllModules() const override;

		//! Add an IProcess to system to update.
		SG_CORE_API virtual void AddIProcess(IProcess* pProcess) override;
		//! Remove an IProcess from system.
		//virtual void RemoveIProcess(IProcess* pProcess) override;

		//! Get current memory usage for all the modules.
		SG_CORE_API virtual UInt32 GetTotalMemoryUsage() const override;
	private:
		enum {
			SG_MAX_FILE_PATH = 200,
			SG_MAX_DRIVE_PATH = 5,
			SG_MAX_FILE_NAME = 30,
			SG_MAX_EXT_PATH = 10,
			SG_MAX_DIREC_PATH = SG_MAX_FILE_PATH - SG_MAX_DRIVE_PATH - SG_MAX_FILE_NAME - SG_MAX_EXT_PATH,
		};

		string mRootPath;

		ISystemModules mSystemModules;
		IProcess* mpCurrActiveProcess;
	};



}