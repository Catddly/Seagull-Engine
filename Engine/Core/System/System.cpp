#include "StdAfx.h"
#include "System.h"

#include "Core/Log/Log.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/System/InputSystem.h"
#include "Core/Platform/OperatingSystem.h"

namespace SG
{
	// no implementation yet, just use a forward declaration
	class C2DEngine;

	System* System::sInstance = nullptr;

	void System::OnInit()
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

	void System::OnShutdown()
	{
		if (mpCurrActiveProcess) mpCurrActiveProcess->OnShutdown();

		mSystemModules.pOS->OnShutdown();
		mSystemModules.pLog->OnShutdown();
		mSystemModules.pFileSystem->OnShutdown();
		delete mSystemModules.pOS;
		delete mSystemModules.pLog;
		delete mSystemModules.pFileSystem;
		delete mSystemModules.pInputSystem;
		mSystemModules.pOS = nullptr;
		mSystemModules.pFileSystem = nullptr;
		mSystemModules.pLog = nullptr;
		mSystemModules.pInputSystem = nullptr;

		if (sInstance)
			delete sInstance;
	}

	void System::SetI3DEngine(I3DEngine* p3DEngine) { mSystemModules.p3DEngine = p3DEngine; }
	void System::SetI2DEngine(I2DEngine* p2DEngine) { mSystemModules.p2DEngine = p2DEngine; }
	void System::SetRenderer(Renderer* pRenderer)   { mSystemModules.pRenderer = pRenderer; }

	SG::SSystemModules*   System::GetSystemModules() { return &mSystemModules; }
	SG::I3DEngine*        System::GetI3DEngine()     { return mSystemModules.p3DEngine; }
	SG::I2DEngine*        System::GetI2DEngine()     { return mSystemModules.p2DEngine; }
	SG::ILog*             System::GetILog()          { return mSystemModules.pLog; }
	SG::IFileSystem*      System::GetIFileSystem()   { return mSystemModules.pFileSystem; }
	SG::IInputSystem*     System::GetIInputSystem()  { return mSystemModules.pInputSystem; }
	SG::IOperatingSystem* System::GetIOS()           { return mSystemModules.pOS; }
	Renderer*             System::GetRenderer()      { return mSystemModules.pRenderer; }

	bool System::ValidateCoreModules() const
	{
		bool isReady = (mSystemModules.pLog != nullptr) &&
			(mSystemModules.pFileSystem != nullptr) &&
			(mSystemModules.pOS != nullptr) &&
			(mSystemModules.pInputSystem != nullptr);
		return isReady;
	}

	bool System::ValidateAllModules() const
	{
		bool isReady = ValidateCoreModules() &&
			(mSystemModules.p2DEngine != nullptr) &&
			(mSystemModules.p3DEngine != nullptr) &&
			(mSystemModules.pRenderer != nullptr);
		return isReady;
	}

	SG::UInt32 System::GetTotalMemoryUsage() const
	{
		// no implementation yet.
		return 0;
	}

	void System::SetRootDirectory(const char* filepath)
	{
		mRootPath = filepath;
		_chdir(filepath);
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
		if (!mSystemModules.pFileSystem)  mSystemModules.pFileSystem = new CFileSystem;
		if (!mSystemModules.pLog)         mSystemModules.pLog = new CLog;
		if (!mSystemModules.pOS)          mSystemModules.pOS = new COperatingSystem;
		if (!mSystemModules.pInputSystem) mSystemModules.pInputSystem = new CInputSystem;
		mSystemModules.pFileSystem->OnInit();
		mSystemModules.pLog->OnInit();
		mSystemModules.pOS->OnInit();
		return ValidateCoreModules();
	}

	System* System::GetInstance()
	{
		if (sInstance == nullptr)
			sInstance = new System;
		return sInstance;
	}

	System::System()
		:mRootPath("")
	{}

	bool System::SystemMainLoop()
	{
		bool bIsSafeQuit = true;
		bool bIsExit = false;
		while (!bIsExit)
		{
			EOsMessage msg = EOsMessage::eNull;
			msg = PeekOSMessage();
			if (msg == EOsMessage::eQuit)
				bIsExit = true;

			Update();
		}
		return bIsSafeQuit;
	}

	void System::Update()
	{
		mSystemModules.pInputSystem->OnUpdate();
		if (mSystemModules.p3DEngine) mSystemModules.p3DEngine->OnUpdate();
		if (mSystemModules.p2DEngine) mSystemModules.p2DEngine->OnUpdate();

		if (mpCurrActiveProcess)      mpCurrActiveProcess->OnUpdate();
	}

}