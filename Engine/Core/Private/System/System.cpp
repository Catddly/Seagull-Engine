#include "StdAfx.h"
#include "System/System.h"

#include "System/FileSystem.h"
#include "System/Logger.h"
#include "System/Input.h"
#include "User/IApp.h"
#include "Event/MessageBus/MessageBus.h"
#include "Archive/Serialization.h"

#include "Render/Shader/ShaderLibrary.h"

#include "Profile/Timer.h"
#include "Profile/FpsTimer.h"
#include "Memory/Memory.h"
#include "Profile/Profile.h"

#include <filesystem>

namespace SG
{

	// no implementation yet, just use a forward declaration
	class C2DEngine;

	void System::Initialize()
	{
		SG_PROFILE_FUNCTION();

#ifdef SG_PLATFORM_WINDOWS
		char abPath[SG_MAX_FILE_PATH] = { 0 };
		::GetModuleFileNameA(NULL, abPath, sizeof(abPath));
		string myProcessPath = abPath;
#else
#	error Can not set the default root path in this platform.
#endif
		Size slashPos = myProcessPath.find_last_of('\\');
		myProcessPath = myProcessPath.substr(0, slashPos);
		SetRootPath(myProcessPath);

		FileSystem::OnInit();
		Logger::OnInit();
		Input::OnInit();
		OperatingSystem::OnInit();

		// set current thread to main thread
		SetCurrThreadName("Main Thread");
		mMainThread.pFunc = nullptr;
		mMainThread.pHandle = nullptr;
		mMainThread.pUser = nullptr;

		if (mpCurrActiveProcess)
			mpCurrActiveProcess->OnInit();

		mp3DScene = MakeRef<Scene>();
		mp3DScene->OnSceneLoad();

		//Deserializer::Deserialize(mp3DScene);

		mpRenderDataBuilder = MakeRef<RenderDataBuilder>();
		mpRenderDataBuilder->SetScene(mp3DScene);
		mpRenderDataBuilder->LoadInNeccessaryDataFromDisk();
		mpRenderDataBuilder->ResolveRenderData();

		ShaderLibrary::GetInstance()->OnInit();
	}

	void System::Shutdown()
	{
		SG_PROFILE_FUNCTION();

		ShaderLibrary::GetInstance()->OnShutdown();
		mp3DScene->OnSceneUnLoad();

		if (mpCurrActiveProcess)
		{
			mpCurrActiveProcess->OnShutdown();
			Delete(mpCurrActiveProcess);
		}

		OperatingSystem::OnShutdown();
		Input::OnShutdown();

		Logger::OnShutdown();
		FileSystem::OnShutdown();
	}

	void System::AddIProcess(IProcess* pProcess)
	{
		SG_PROFILE_FUNCTION();

		if (pProcess)
			mpCurrActiveProcess = pProcess;
	}

	bool System::SystemMainLoop()
	{
		SG_PROFILE_FUNCTION();

		bool bIsSafeQuit = true;
		bool bIsExit = false;

		FpsTimer gameloopTimer("GameLoop", 0.5f);
		while (!bIsExit)
		{
			gameloopTimer.ProfileScope();
			float deltaTime = gameloopTimer.GetDurationSecond();

			{
				SG_PROFILE_SCOPE("Message Peeking");

				// collect all the messages
				EOsMessage msg = EOsMessage::eNull;
				msg = PeekOSMessage();
				if (msg == EOsMessage::eQuit)
					bIsExit = true;
			}

			// dispatch all the system messages
			mSystemMessageManager.Update();
			Input::OnUpdate(deltaTime);

			mp3DScene->OnUpdate(deltaTime);

			// modules OnUpdate()
			mModuleManager.Update(deltaTime);
			if (mpCurrActiveProcess)
				mpCurrActiveProcess->OnUpdate(deltaTime);

			// modules OnDraw()
			mModuleManager.Draw();
			if (mpCurrActiveProcess)
				mpCurrActiveProcess->OnDraw();

			Impl::MessageBus::GetInstance()->ClearEvents();

			SG_PROFILE_FRAME_MARK();
		}
		return bIsSafeQuit;
	}

	bool System::ValidateModules() const
	{
		//bool isReady = ValidateCoreModules() &&
		//	(mSystemModules.p2DEngine != nullptr) &&
		//	(mSystemModules.p3DEngine != nullptr) &&
		//	(mSystemModules.pRenderer != nullptr);
		return false;
	}

	RefPtr<Scene> System::GetMainScene()
	{
		SG_PROFILE_FUNCTION();

		return mp3DScene;
	}

	RefPtr<RenderDataBuilder> System::GetRenderDataBuilder()
	{
		SG_PROFILE_FUNCTION();

		return mpRenderDataBuilder;
	}

	UInt32 System::GetTotalMemoryUsage() const
	{
		// no implementation yet.
		return 0;
	}

	void System::SetRootPath(const string& path)
	{
		SG_PROFILE_FUNCTION();

		std::filesystem::current_path(path.c_str());
	}

	int System::RunProcess(const string& command, const char* pOut)
	{
		SG_PROFILE_FUNCTION();

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

	void System::RegisterSystemMessageListener(ISystemMessageListener* pListener)
	{
		SG_PROFILE_FUNCTION();

		mSystemMessageManager.RegisterListener(pListener);
	}

	void System::RemoveSystemMessageListener(ISystemMessageListener* pListener)
	{
		SG_PROFILE_FUNCTION();

		mSystemMessageManager.RemoveListener(pListener);
	}

	void System::Terminate()
	{
#ifdef SG_PLATFORM_WINDOWS
		::PostQuitMessage(0);
#endif
	}

	System* const System::GetInstance()
	{
		SG_PROFILE_FUNCTION();

		static System instance;
		return &instance;
	}

}