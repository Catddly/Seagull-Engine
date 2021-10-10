#include "StdAfx.h"
#include "System/System.h"

#include "Core/Private/Logger/Logger.h"
#include "Core/Private/FileSystem/FileSystem.h"
#include "Core/Private/System/InputSystem.h"
#include "Core/Private/Platform/OperatingSystem.h"

#include "User/IApp.h"

#include "Profile/FpsTimer.h"
#include "Memory/IMemory.h"

#include <windows.h>

namespace SG
{
	// no implementation yet, just use a forward declaration
	class C2DEngine;

	System::System()
		:mRootPath("")
	{}

	void System::Initialize()
	{
		char abPath[SG_MAX_FILE_PATH] = { 0 };
		::GetModuleFileNameA(NULL, abPath, sizeof(abPath));
		char drivePath[SG_MAX_DRIVE_PATH] = { 0 };
		char directoryPath[SG_MAX_DIREC_PATH] = { 0 };
		_splitpath_s(abPath,
			drivePath, SG_MAX_DRIVE_PATH,
			directoryPath, SG_MAX_DIREC_PATH, NULL, 0, NULL, 0);

		string tempRootPath(drivePath);
		tempRootPath.append(directoryPath);
		mRootPath = move(tempRootPath);
		// set root directory to where the .exe file is
		_chdir(mRootPath.c_str());

		// set current thread to main thread
		SetCurrThreadName("Main Thread");
		mMainThread.pFunc = nullptr;
		mMainThread.pHandle = nullptr;
		mMainThread.pUser = nullptr;

		if (!InitCoreModules())
			SG_LOG_ERROR("Failed to initialized core modules");
	}

	void System::Shutdown()
	{
		if (mpCurrActiveProcess) mpCurrActiveProcess->OnShutdown();
		Memory::Delete(mpCurrActiveProcess);
	}

	bool System::ValidateCoreModules() const
	{
		// TODO: fix it
		//bool isReady = (mSystemModules.pLog != nullptr) &&
		//	(mSystemModules.pFileSystem != nullptr) &&
		//	(mSystemModules.pOS != nullptr) &&
		//	(mSystemModules.pInputSystem != nullptr);
		return true;
	}

	bool System::ValidateAllModules() const
	{
		//bool isReady = ValidateCoreModules() &&
		//	(mSystemModules.p2DEngine != nullptr) &&
		//	(mSystemModules.p3DEngine != nullptr) &&
		//	(mSystemModules.pRenderer != nullptr);
		return false;
	}

	SG::UInt32 System::GetTotalMemoryUsage() const
	{
		// no implementation yet.
		return 0;
	}

	int System::RunProcess(const string& command, const char* pOut)
	{
#ifdef SG_PLATFORM_WINDOWS
		STARTUPINFOA        startupInfo;
		PROCESS_INFORMATION processInfo;
		memset(&startupInfo, 0, sizeof startupInfo);
		memset(&processInfo, 0, sizeof processInfo);

		HANDLE stdOut = NULL;
		if (pOut)
		{
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(sa);
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle = TRUE;

			size_t   pathLength = strlen(pOut) + 1;
			wchar_t* buffer = (wchar_t*)alloca(pathLength * sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, pOut, (int)pathLength, buffer, (int)pathLength);
			stdOut = CreateFileW(buffer, GENERIC_ALL, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		}

		startupInfo.cb = sizeof(STARTUPINFO);
		startupInfo.dwFlags |= STARTF_USESTDHANDLES;
		startupInfo.hStdOutput = stdOut;
		startupInfo.hStdError  = stdOut;

		// create a process
		if (!CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, stdOut ? TRUE : FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo))
			return -1;

		WaitForSingleObject(processInfo.hProcess, INFINITE);
		DWORD exitCode;
		GetExitCodeProcess(processInfo.hProcess, &exitCode);

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);

		if (stdOut)
			CloseHandle(stdOut);
#endif
		return EXIT_SUCCESS;
	}

	void System::AddIProcess(IProcess* pProcess)
	{
		if (pProcess)
		{
			mpCurrActiveProcess = pProcess;
			pProcess->OnInit();
		}
	}

	bool System::InitCoreModules()
	{
		IFileSystem* pFileSystem = Memory::New<FileSystem>();
		ILogger* pLogger = Memory::New<CLogger>();
		IOperatingSystem* pOS = Memory::New<COperatingSystem>();
		IInputSystem* pInputSystem = Memory::New<InputSystem>();

		mModuleManager.RegisterCoreModule(pFileSystem);
		mModuleManager.RegisterCoreModule(pLogger);
		mModuleManager.RegisterCoreModule(pOS);
		mModuleManager.RegisterCoreModule(pInputSystem);

		return ValidateCoreModules();
	}

	bool System::SystemMainLoop()
	{
		bool bIsSafeQuit = true;
		bool bIsExit = false;

		FpsTimer renderTimer("RenderDevice::OnDraw()", 3.0f, 60);
		
		while (!bIsExit)
		{
			// collect all the messages
			EOsMessage msg = EOsMessage::eNull;
			msg = PeekOSMessage();
			if (msg == EOsMessage::eQuit)
				bIsExit = true;
			// dispatch all the system messages
			mMessageBus.Update();
			
			// modules OnUpdate()
			mModuleManager.Update();
			if (mpCurrActiveProcess)
				mpCurrActiveProcess->OnUpdate();

			// modules OnDraw()
			{
				renderTimer.BeginProfile();
				mModuleManager.Draw();
				renderTimer.EndProfile();
			}
		}
		return bIsSafeQuit;
	}

	string System::GetResourceDirectory(EResourceDirectory rd) const
	{
		switch (rd)
		{
		case EResourceDirectory::eRoot:             return mRootPath;
		case EResourceDirectory::eShader_Binarires: return mRootPath + "ShaderBin\\";
		case EResourceDirectory::eShader_Sources:   return mRootPath + "ShaderSrc\\";
		case EResourceDirectory::eMeshes:           return mRootPath + "Mesh\\";
		case EResourceDirectory::eTextures:         return mRootPath + "Texture\\";
		case EResourceDirectory::eFonts:            return mRootPath + "Font\\";
		case EResourceDirectory::eLog:              return mRootPath + "Log\\";
		case EResourceDirectory::eScripts:          return mRootPath + "Script\\";
		default: return "";
		}
	}

	void System::SetRootDirectory(const char* filepath)
	{
		mRootPath = filepath;
		_chdir(filepath);
	}

	ILogger*          System::GetLogger()      const { return mModuleManager.GetModule<ILogger*>("Logger", true); }
	IFileSystem*      System::GetFileSystem()  const { return mModuleManager.GetModule<IFileSystem*>("FileSystem", true); }
	IInputSystem*     System::GetInputSystem() const { return mModuleManager.GetModule<IInputSystem*>("InputSystem", true); }
	IOperatingSystem* System::GetOS()          const { return mModuleManager.GetModule<IOperatingSystem*>("OS", true); }

	System* const System::Instance()
	{
		static System instance;
		return &instance;
	}

	void System::RegisterSystemMessageListener(ISystemMessageListener* pListener)
	{
		mMessageBus.RegisterListener(pListener);
	}

	void System::RemoveSystemMessageListener(ISystemMessageListener* pListener)
	{
		mMessageBus.RemoveListener(pListener);
	}

}