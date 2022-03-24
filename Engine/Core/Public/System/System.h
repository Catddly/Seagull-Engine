#pragma once

#include "Core/Config.h"
#include "Thread/Thread.h"

#include "System/FileSystem.h"
#include "Scene/Scene.h"
#include "Platform/OS.h"
#include "Core/Private/System/ModuleManager.h"

#include "Reflection/Name.h"

#include "Stl/string.h"
#include "Stl/SmartPtr.h"
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

		//! Check if all the modules is loaded.
		SG_CORE_API bool ValidateModules() const;

		SG_CORE_API Scene* GetMainScene();

		//! Get current memory usage for all the modules.
		SG_CORE_API UInt32 GetTotalMemoryUsage() const;

		SG_CORE_API int  RunProcess(const string& command, const char* pOut);

		SG_CORE_API void RegisterSystemMessageListener(ISystemMessageListener* pListener);
		SG_CORE_API void RemoveSystemMessageListener(ISystemMessageListener* pListener);

		template <class T>
		bool RegisterModule();

		template <class T>
		void UnResgisterModule();

		template <class T>
		T GetModule() const;

		//! Force to use ISystemManager as the interface of system manager.
		SG_CORE_API static System* const Instance();
	private:
		friend class Main;

		void Initialize();
		void Shutdown();

		//! Add an IProcess to system to update.
		void AddIProcess(IProcess* pProcess);

		// TODO: should not be here, move to Main or something
		bool SystemMainLoop();
	private:
		System() = default;
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
		string        mRootPath = "";
		// TODO: add a 3DWorld can contain a lot of scenes.
		UniquePtr<Scene> mp3DScene = nullptr;
	};

	template <class T>
	void System::UnResgisterModule()
	{
		IModule* pModule = mModuleManager.UnRegisterUserModule(Refl::CT_TypeName<T>());
		Memory::Delete(pModule);
	}

	template <class T>
	bool System::RegisterModule()
	{
		T* pModule = Memory::New<T>();
		if (pModule)
		{
			mModuleManager.RegisterUserModule(Refl::CT_TypeName<T>(), pModule);
			return true;
		}
		return false;
	}

	template <class T>
	T System::GetModule() const
	{
		return mModuleManager.GetModule<T>();
	}

#define SSystem() System::Instance()

}