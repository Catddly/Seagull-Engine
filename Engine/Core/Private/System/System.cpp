#include "StdAfx.h"
#include "System.h"

#include "Core/Private/Log/Logger.h"
#include "Core/Private/FileSystem/FileSystem.h"
#include "Core/Private/System/InputSystem.h"
#include "Core/Private/Platform/OperatingSystem.h"

#include "Memory/IMemory.h"

#include <windows.h>

namespace SG
{
	// no implementation yet, just use a forward declaration
	class C2DEngine;

	CSystem* CSystem::sInstance = nullptr;

	CSystem::CSystem()
		:mRootPath("")
	{}

	void CSystem::OnInit()
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

	void CSystem::OnShutdown()
	{
		if (mpCurrActiveProcess) mpCurrActiveProcess->OnShutdown();
		Memory::Delete(mpCurrActiveProcess);

		if (sInstance)
			Memory::Delete(sInstance);
	}

	bool CSystem::ValidateCoreModules() const
	{
		// TODO: fix it
		//bool isReady = (mSystemModules.pLog != nullptr) &&
		//	(mSystemModules.pFileSystem != nullptr) &&
		//	(mSystemModules.pOS != nullptr) &&
		//	(mSystemModules.pInputSystem != nullptr);
		return true;
	}

	bool CSystem::ValidateAllModules() const
	{
		//bool isReady = ValidateCoreModules() &&
		//	(mSystemModules.p2DEngine != nullptr) &&
		//	(mSystemModules.p3DEngine != nullptr) &&
		//	(mSystemModules.pRenderer != nullptr);
		return false;
	}

	SG::UInt32 CSystem::GetTotalMemoryUsage() const
	{
		// no implementation yet.
		return 0;
	}

	int CSystem::RunProcess(const char* pCommand, const char** ppArgs, Size argNum, const char* pOut)
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

		string commandLine = pCommand;
		for (int i = 0; i < argNum; i++)
			commandLine += " " + string(ppArgs[i]);

		// create a process
		if (!CreateProcessA(NULL, (LPSTR)commandLine.c_str(), NULL, NULL, stdOut ? TRUE : FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo))
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

	void CSystem::AddIProcess(IProcess* pProcess)
	{
		if (pProcess)
		{
			mpCurrActiveProcess = pProcess;
			pProcess->OnInit();
		}
	}

	bool CSystem::InitCoreModules()
	{
		IFileSystem* pFileSystem = Memory::New<CFileSystem>();
		ILogger* pLogger = Memory::New<CLogger>();
		IOperatingSystem* pOS = Memory::New<COperatingSystem>();
		IInputSystem* pInputSystem = Memory::New<CInputSystem>();

		mModuleManager.RegisterCoreModule(pFileSystem);
		mModuleManager.RegisterCoreModule(pLogger);
		mModuleManager.RegisterCoreModule(pOS);
		mModuleManager.RegisterCoreModule(pInputSystem);

		return ValidateCoreModules();
	}

	bool CSystem::RegisterModule(IModule* pModule)
	{
		if (pModule)
		{
			mModuleManager.RegisterUserModule(pModule);
			return true;
		}
		return false;
	}

	bool CSystem::SystemMainLoop()
	{
		bool bIsSafeQuit = true;
		bool bIsExit = false;
		while (!bIsExit)
		{
			EOsMessage msg = EOsMessage::eNull;
			msg = PeekOSMessage();
			if (msg == EOsMessage::eQuit)
				bIsExit = true;

			OnUpdate();
		}
		return bIsSafeQuit;
	}

	void CSystem::OnUpdate()
	{
		mModuleManager.OnUpdate();
		if (mpCurrActiveProcess) 
			mpCurrActiveProcess->OnUpdate();
	}

	string CSystem::GetResourceDirectory(EResourceDirectory rd) const
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

	CSystem* CSystem::GetInstance()
	{
		if (sInstance == nullptr)
			sInstance = Memory::New<CSystem>();
		return sInstance;
	}

	void CSystem::SetRootDirectory(const char* filepath)
	{
		mRootPath = filepath;
		_chdir(filepath);
	}

	ILogger*          CSystem::GetLogger()      const { return mModuleManager.GetModule<ILogger*>("Logger", true); }
	IFileSystem*      CSystem::GetFileSystem()  const { return mModuleManager.GetModule<IFileSystem*>("FileSystem", true); }
	IInputSystem*     CSystem::GetInputSystem() const { return mModuleManager.GetModule<IInputSystem*>("InputSystem", true); }
	IOperatingSystem* CSystem::GetOS()          const { return mModuleManager.GetModule<IOperatingSystem*>("OS", true); }

}