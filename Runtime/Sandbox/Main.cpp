#include "StdAfx.h"

class MyApp : public SG::IApp
{
public:
	virtual void OnInit() override
	{
		SG_LOG_INFO("User OnInit()");
		using namespace SG;

		//MathTest();
		//ThreadTest();
	}

	virtual void OnUpdate() override
	{
	}

	virtual void OnShutdown() override
	{
		SG_LOG_INFO("User OnExit()");
	}
private:
	static void _ThreadFunc(void* pUser)
	{
		using namespace SG;
		SetCurrThreadName("Worker Thread");
		{
			ScopeLock lck(sMutex);
			++sCounter;
			SG_LOG_DEBUG("Current thread id: %d, counter: %d", 
				GetCurrThreadID(), sCounter);
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

		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, mat3f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, mat4f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, mat3i);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, mat4i);

		Vector2f vec2f = Vector2f::Identity();
		Vector3f vec3f = Vector3f::Identity();
		Vector4f vec4f = Vector4f::Identity();
		Vector2i vec2i = Vector2i::Identity();
		Vector3i vec3i = Vector3i::Identity();
		Vector4i vec4i = Vector4i::Identity();

		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec2f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec3f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec4f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec2i);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec3i);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec4i);
	}

	void ThreadTest()
	{
		using namespace SG;
		UInt32 numCores = GetNumCPUCores();
		SG_LOG_DEBUG("Current CPU cores: (%d)", numCores);
		Thread threads[8] = {};
		for (int i = 0; i < 8; i++)
		{
			CreThread(&threads[i], _ThreadFunc, nullptr);
		}

		for (int i = 0; i < 8; i++)
		{
			JoinThread(&threads[i]);
		}
	}
private:
	static SG::Mutex  sMutex;
	static SG::UInt32 sCounter;
};

SG::Mutex  MyApp::sMutex;
SG::UInt32 MyApp::sCounter = 0;

SG::IApp* SG::GetAppInstance()
{
	return new MyApp;
}