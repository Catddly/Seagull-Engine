#include "StdAfx.h"

SG::ConditionVariable gCv;
SG::Mutex             gMutex;
SG::Mutex             gLogMutex;
SG::Atomic32          gCount;
void CvThreadFunc(void* pUser)
{
	using namespace SG;
	SetCurrThreadName("Cv Thread");
	gCv.Wait(gMutex);

	{
		ScopeLock lck(gLogMutex);
		SG_LOG_DEBUG("Func going on!");
	}
}

class MyApp : public SG::IApp, public SG::IInputListener
{
public:
	virtual void OnInit() override
	{
		SG_LOG_INFO("User OnInit()");
		using namespace SG;

		//auto* pInputSystem = SG::System::GetInstance()->GetIInputSystem();
		//pInputSystem->RegisterListener(this);

		struct MyJob : public SG::IJob<int, double>
		{
			virtual void Execute(const int& inData, double& outData) override
			{
				outData = (double)inData + 5.99;
			}
		};

		MyJob job;
		int a = 54;
		double b;
		job.Execute(a, b);
		SG_LOG_INFO("b is %.2llf", b);

		//MathTest();
		//ThreadTest();
	}

	virtual void OnUpdate(float deltaTime) override
	{
		using namespace SG;

		//auto* pOS = CSystem::GetInstance()->GetOS();
		//Window* pMainWindow = pOS->GetMainWindow();
		//Rect& pWindowRect = pMainWindow->GetCurrRect();
		//Monitor* pMainMOnitor = pOS->GetMainMonitor();
		//Vector2i pos = pMainWindow->GetMousePosRelative();
		//Vector2i pos2 = pOS->GetMousePos();
		//SG_LOG_DEBUG("Window Rect (%d, %d) (%d, %d)", pWindowRect.left, pWindowRect.top, pWindowRect.right, pWindowRect.bottom);
		//SG_LOG_DEBUG("Mouse Pos(Relative) (%d, %d)", pos[0], pos[1]);
		//SG_LOG_DEBUG("Mouse Pos(R - A) (%d, %d)", pos[0] - pos2[0], pos[1] - pos2[1]);
		//SG_LOG_MATH(ELogLevel::eLog_Level_Debug, pos2);

		//SG_LOG_IF(ELogLevel::eLog_Level_Debug, "Is Window Out of screen: ", pOS->IsMainWindowOutOfScreen());
	}

	virtual void OnShutdown() override
	{
		//auto* pInputSystem = SG::System::GetInstance()->GetIInputSystem();
		//pInputSystem->RemoveListener(this);

		SG_LOG_INFO("User OnExit()");
	}

	virtual bool OnInputUpdate(SG::EKeyCode keycode, SG::EKeyState keyState, int xPos, int yPos) override
	{
		SG_LOG_INFO("User input %d (%d)", keycode, keyState);
		return true;
	}
private:
	static void _ThreadFunc(void* pUser)
	{
		using namespace SG;
		SetCurrThreadName("Worker Thread");
		{
			ScopeLock lck(gLogMutex);
			SG_LOG_DEBUG("Current thread id: %d, counter: %d", 
				GetCurrThreadID(), gCount.Increase());
		}
	}
private:
	void MathTest()
	{
		using namespace SG;
		Matrix4f mat4f = Matrix4f::Identity();
		Matrix3f mat3f = Matrix3f::Identity();
		Matrix3i mat3i = Matrix3i::Identity();
		Matrix4i mat4i = Matrix4i::Identity();

		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, mat3f, "");
		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, mat4f, "");
		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, mat3i, "");
		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, mat4i, "");

		Vector2f vec2f = Vector2f::Identity();
		Vector3f vec3f = Vector3f::Identity();
		Vector4f vec4f = Vector4f::Identity();
		Vector2i vec2i = Vector2i::Identity();
		Vector3i vec3i = Vector3i::Identity();
		Vector4i vec4i = Vector4i::Identity();

		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, vec2f, "");
		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, vec3f, "");
		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, vec4f, "");
		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, vec2i, "");
		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, vec3i, "");
		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, vec4i, "");
	}

	void ThreadTest()
	{
		using namespace SG;
		UInt32 numCores = GetNumCPUCores();
		SG_LOG_DEBUG("Current CPU cores: (%d)", numCores);
		Thread threads[8] = {};
		for (int i = 0; i < 8; i++)
		{
			ThreadCreate(&threads[i], _ThreadFunc, nullptr);
		}

		Thread cvThread = {};
		ThreadCreate(&cvThread, CvThreadFunc, nullptr);

		for (int i = 0; i < 8; i++)
		{
			ThreadJoin(&threads[i]);
		}

		gCv.NotifyOne();

		ThreadJoin(&cvThread);
	}

	void FileSystemTest()
	{
		using namespace SG;
		auto pIO = SSystem()->GetFileSystem();
		if (pIO->Open(EResourceDirectory::eRoot, "test.spv", EFileMode::efWrite_Binary))
		{
			int a = 4;
			pIO->Write(&a, 4);
			pIO->Close();
		}

		if (pIO->Open(EResourceDirectory::eRoot, "test.spv", EFileMode::efRead_Binary))
		{
			SG_LOG_DEBUG("FileSize: %d", pIO->FileSize());
			void* buf = Memory::Malloc(pIO->FileSize());
			pIO->Read(buf, pIO->FileSize());
			SG_LOG_DEBUG("a = %d", *(int*)buf);
			pIO->Close();
			Memory::Free(buf);
		}
	}
private:
	static SG::Mutex  sMutex;
	SG::VulkanInstance* mVkInstance = nullptr;
};

SG::Mutex  MyApp::sMutex;

SG::IApp* SG::GetAppInstance()
{
	return Memory::New<MyApp>();
}