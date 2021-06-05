#pragma once

#include "Core/Config.h"
#include "Common/Base/ISingleton.h"
#include "Common/System/ISystem.h"

#include "Common/System/ILog.h"
#include "Common/System/I2DEngine.h"
#include "Common/System/I3DEngine.h"
#include "Common/System/IFileSystem.h"

#include "Common/User/IApp.h"

namespace SG
{

	class SG_CORE_API CSystemManager final : public ISystemManager, public ISingleton<CSystemManager>
	{
	public:
		CSystemManager();
		~CSystemManager() = default;

		virtual bool TryInitCoreModules() override;
		virtual void Update() override; // do we really want this??
		virtual void Shutdown() override;

		virtual ISystemModules* GetSystemModules() override;

		virtual void       SetI3DEngine(I3DEngine* p3DEngine) override;
		virtual I3DEngine* GetI3DEngine() override;
		virtual void       SetI2DEngine(I2DEngine* p3DEngine) override;
		virtual I2DEngine* GetI2DEngine() override;

		virtual ILog* GetILog() override;
		virtual IFileSystem* GetIFileSystem() override;

		//! Register a user application.
		virtual void RegisterUserApp(IApp* pApp) override;

		//! Check if all the core modules is loaded.
		virtual bool ValidateCoreModules() const override;
		//! Check if all the modules is loaded.
		virtual bool ValidateAllModules() const override;

		//! Add an IProcess to system to update.
		virtual void AddIProcess(IProcess* pProcess) override;
		//! Remove an IProcess from system.
		virtual void RemoveIProcess(IProcess* pProcess) override;

		//! Get current memory usage for all the modules.
		virtual UInt32 GetTotalMemoryUsage() const override;
	private:
		ISystemModules mSystemModules;
		IApp* mpUserApp;
	};

}