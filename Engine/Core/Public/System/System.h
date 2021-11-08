#pragma once

#include "Core/Config.h"
#include "Thread/Thread.h"

#include "System/FileSystem.h"
#include "Platform/OS.h"
#include "Core/Private/System/ModuleManager.h"

#include "Stl/string.h"
#include <eastl/set.h>

#ifdef SG_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	endif
#endif

namespace SG
{
	interface IProcess;

	enum class ESystemMessage
	{
		eWindowResize = 0,
		eWindowMove,
		eWindowMinimal,
		eTerminate,
	};

	interface SG_CORE_API ISystemMessageListener
	{
		virtual bool OnSystemMessage(ESystemMessage msg) = 0;
	};

	class SystemMessageBus
	{
	public:
		void Update();

		void RegisterListener(ISystemMessageListener* pListener);
		void RemoveListener(ISystemMessageListener* pListener);

		void PushEvent(ESystemMessage msg);
	private:
		eastl::set<ESystemMessage>          mMessages;
		eastl::set<ISystemMessageListener*> mpListeners;
	};

	class System
	{
	public:
		~System() = default;

		SG_CORE_API void Initialize();
		SG_CORE_API void Shutdown();

		//! Check if all the modules is loaded.
		SG_CORE_API bool ValidateModules() const;

		SG_CORE_API bool SystemMainLoop();

		//! Add an IProcess to system to update.
		SG_CORE_API void AddIProcess(IProcess* pProcess);
		//! Remove an IProcess from system.
		//virtual void RemoveIProcess(IProcess* pProcess) override;

		//! Get current memory usage for all the modules.
		SG_CORE_API UInt32 GetTotalMemoryUsage() const;
		//! Set engine's resource files root directory in absolute path.
		//! Default root directory will be the folder where .exe is in.
		//! \param (filepath) relative path to the .exe
		SG_CORE_API void        SetRootDirectory(const char* filepath);
		SG_CORE_API string      GetResourceDirectory(EResourceDirectory rd) const;

		SG_CORE_API int RunProcess(const string& command, const char* pOut);

		SG_CORE_API void RegisterSystemMessageListener(ISystemMessageListener* pListener);
		SG_CORE_API void RemoveSystemMessageListener(ISystemMessageListener* pListener);

		//! Force to use ISystemManager as the interface of system manager.
		SG_CORE_API static System* const Instance();

		template <class T>
		bool RegisterModule();

		template <class T>
		void UnResgisterModule();

		template <class T>
		T GetModule(const char* name) const;
	private:
		System();
#ifdef SG_PLATFORM_WINDOWS
		friend LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
	private:
		enum {
			SG_MAX_FILE_PATH = 200,
			SG_MAX_DRIVE_PATH = 5,
			SG_MAX_FILE_NAME = 30,
			SG_MAX_EXT_PATH = 10,
			SG_MAX_DIREC_PATH = SG_MAX_FILE_PATH - SG_MAX_DRIVE_PATH - SG_MAX_FILE_NAME - SG_MAX_EXT_PATH,
		};

		ModuleManager    mModuleManager;
		SystemMessageBus mMessageBus;

		IProcess*     mpCurrActiveProcess;
		Thread        mMainThread;
		string        mRootPath;
	};

	template <class T>
	void SG::System::UnResgisterModule()
	{
		IModule* pModule = mModuleManager.UnRegisterUserModule(T::GetModuleName());
		Memory::Delete(pModule);
	}

	template <class T>
	bool SG::System::RegisterModule()
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
	T SG::System::GetModule(const char* name) const
	{
		return mModuleManager.GetModule<T>(name);
	}

#define SSystem() System::Instance()

}