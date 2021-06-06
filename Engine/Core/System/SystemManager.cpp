#include "StdAfx.h"
#include "SystemManager.h"

#include "Core/Memory/Memory.h"

#include "Core/Log/Log.h"
#include "Core/FileSystem/FileSystem.h"

namespace SG
{
	// no implementation yet, just forward declaration
	class C2DEngine;

	CSystemManager::CSystemManager()
		: mpCurrActiveProcess(nullptr), mRootPath("")
	{}

	void CSystemManager::InitSystemEnv()
	{
		// set root directory to where the .exe file is
		// TODO: move to SEnv or something control all the environment variable
		char abPath[SG_MAX_FILE_PATH] = { 0 };
		GetModuleFileNameA(NULL, abPath, sizeof(abPath));
		char drivePath[SG_MAX_DRIVE_PATH] = { 0 };
		char directoryPath[SG_MAX_DIREC_PATH] = { 0 };
		_splitpath_s(abPath,
			drivePath, SG_MAX_DRIVE_PATH,
			directoryPath, SG_MAX_DIREC_PATH, NULL, 0, NULL, 0);

		string rootPath(drivePath);
		rootPath.append(directoryPath);
		mRootPath = move(rootPath);
		_chdir(mRootPath.c_str());
	}

	void CSystemManager::SetI3DEngine(I3DEngine* p3DEngine) { mSystemModules.p3DEngine = p3DEngine; }
	void CSystemManager::SetI2DEngine(I2DEngine* p2DEngine) { mSystemModules.p2DEngine = p2DEngine; }

	SG::ISystemModules* CSystemManager::GetSystemModules() { return &mSystemModules; }
	SG::I3DEngine*      CSystemManager::GetI3DEngine() { return mSystemModules.p3DEngine; }
	SG::I2DEngine*      CSystemManager::GetI2DEngine() { return mSystemModules.p2DEngine; }
	SG::ILog*           CSystemManager::GetILog() { return mSystemModules.pLog; }
	SG::IFileSystem*    CSystemManager::GetIFileSystem() { return mSystemModules.pFileSystem; }

	bool CSystemManager::ValidateCoreModules() const
	{
		bool isReady = (mSystemModules.pLog != nullptr) &&
			(mSystemModules.pFileSystem != nullptr);
		return isReady;
	}

	bool CSystemManager::ValidateAllModules() const
	{
		bool isReady = ValidateCoreModules() &&
			(mSystemModules.p2DEngine != nullptr) &&
			(mSystemModules.p3DEngine != nullptr);
		return isReady;
	}

	void CSystemManager::AddIProcess(IProcess* pProcess)
	{
		mpCurrActiveProcess = pProcess;
		mpCurrActiveProcess->OnInit();
	}

	SG::UInt32 CSystemManager::GetTotalMemoryUsage() const
	{
		// no implementation yet.
		return 0;
	}

	bool CSystemManager::InitCoreModules()
	{
		if (!mSystemModules.pFileSystem) mSystemModules.pFileSystem = New<CFileSystem>();
		if (!mSystemModules.pLog)        mSystemModules.pLog = New<CLog>();
		mSystemModules.pFileSystem->OnInit();
		mSystemModules.pLog->OnInit();
		return ValidateCoreModules();
	}

	void CSystemManager::Update()
	{
		if (mpCurrActiveProcess) mpCurrActiveProcess->OnUpdate();
	}

	void CSystemManager::Shutdown()
	{
		if (mpCurrActiveProcess) mpCurrActiveProcess->OnShutdown();

		mSystemModules.pLog->OnShutdown();
		Delete(mSystemModules.pLog);
		Delete(mSystemModules.pFileSystem);
		mSystemModules.pFileSystem = nullptr;
		mSystemModules.pLog = nullptr;
	}

	SG_CORE_API CSystemManager gSystemManager;
	ISystemManager* GetSystemManager()
	{
		return &gSystemManager;
	}


}