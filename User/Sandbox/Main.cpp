#include "StdAfx.h"

namespace SG
{
	extern SSystemEnvironment __declspec(dllimport) gEnv;
	SSystemEnvironment gEnv;
}

class MyApp : public SG::IApp
{
public:
	virtual void OnInit() override
	{
		SG_LOG_INFO("User OnInit()");
	}

	virtual void OnUpdate() override
	{
		SG_LOG_INFO("User OnUpdate()");
	}

	virtual void OnExit() override
	{
		SG_LOG_INFO("User OnExit()");
	}
};

SG::IApp* SG::GetAppInstance()
{
	return new MyApp();
}