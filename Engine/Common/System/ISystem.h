#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

#include "Common/Stl/string.h"

namespace SG
{
	struct IProcess;

	struct I3DEngine;
	struct I2DEngine;
	struct ILog;
	struct IFileSystem;
	struct IInputSystem;
	struct IOperatingSystem;

	struct Renderer;

	//! @Interface 
	//! All the system components are in here
	//! We can dynamically change its implementation of modules
	struct SSystemModules
	{
		I3DEngine*        p3DEngine = nullptr;
		I2DEngine*        p2DEngine = nullptr;

		ILog*             pLog = nullptr;
		IFileSystem*      pFileSystem = nullptr;
		IOperatingSystem* pOS = nullptr;
		IInputSystem*     pInputSystem = nullptr;

		Renderer*         pRenderer = nullptr;
	};

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

		virtual SSystemModules*   GetSystemModules() = 0;
		virtual void              SetI3DEngine(I3DEngine* p3DEngine) = 0;
		virtual I3DEngine*        GetI3DEngine() = 0;
		virtual void              SetI2DEngine(I2DEngine* p2DEngine) = 0;
		virtual I2DEngine*        GetI2DEngine() = 0;
		virtual ILog*             GetILog() = 0;
		virtual IFileSystem*      GetIFileSystem() = 0;
		virtual IInputSystem*     GetIInputSystem() = 0;
		virtual IOperatingSystem* GetIOS() = 0;

		virtual void              SetRenderer(Renderer* pRenderer) = 0;
		virtual Renderer*         GetRenderer() = 0;

		//! Register a user application.
		//virtual void RegisterUserApp(IApp* pApp) = 0;
		
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
		virtual void   SetRootDirectory(const char* filepath) = 0;
	};

}