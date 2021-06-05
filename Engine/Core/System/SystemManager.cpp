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
		: mpUserApp(nullptr)
	{}

	void CSystemManager::SetI3DEngine(I3DEngine* p3DEngine) { mSystemModules.p3DEngine = p3DEngine; }
	void CSystemManager::SetI2DEngine(I2DEngine* p2DEngine) { mSystemModules.p2DEngine = p2DEngine; }

	SG::ISystemModules* CSystemManager::GetSystemModules() { return &mSystemModules; }
	SG::I3DEngine*      CSystemManager::GetI3DEngine() { return mSystemModules.p3DEngine; }
	SG::I2DEngine*      CSystemManager::GetI2DEngine() { return mSystemModules.p2DEngine; }
	SG::ILog*           CSystemManager::GetILog() { return mSystemModules.pLog; }
	SG::IFileSystem*    CSystemManager::GetIFileSystem() { return mSystemModules.pFileSystem; }

	void CSystemManager::RegisterUserApp(IApp* pApp)
	{
		mpUserApp = pApp;
		mpUserApp->OnInit();
	}

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
		// no implementation yet.
	}

	void CSystemManager::RemoveIProcess(IProcess* pProcess)
	{
		// no implementation yet.
	}

	SG::UInt32 CSystemManager::GetTotalMemoryUsage() const
	{
		// no implementation yet.
		return 0;
	}

	bool CSystemManager::TryInitCoreModules()
	{
		if (!mSystemModules.pFileSystem) mSystemModules.pFileSystem = New<CFileSystem>();
		if (!mSystemModules.pLog)        mSystemModules.pLog = New<CLog>();
		return ValidateCoreModules();
	}

	void CSystemManager::Update()
	{
		if (mpUserApp) mpUserApp->OnUpdate();
	}

	void CSystemManager::Shutdown()
	{
		if (mpUserApp) mpUserApp->OnShutdown();

		Delete(mSystemModules.pLog);
		Delete(mSystemModules.pFileSystem);
		mSystemModules.pFileSystem = nullptr;
		mSystemModules.pLog = nullptr;
	}

}