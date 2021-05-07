#include "StdAfx.h"

#include "Engine.h"
#include "Common/System/ILog.h"
#include "Common/System/ISystem.h"

namespace SG
{

	extern SSystemEnvironment __declspec(dllimport) gEnv;
	SSystemEnvironment gEnv;

	void CEngine::OnInit()
	{
		SG_LOG_INFO("Engine OnInit()");
	}

	void CEngine::OnUpdate()
	{
		SG_LOG_INFO("Engine OnUpdate()");
	}

	void CEngine::OnExit()
	{
		SG_LOG_INFO("Engine OnExit()");
	}

	void CEngine::GetMainThreadId() const
	{
		SG_LOG_INFO("Engine GetMainThreadId()");
	}

}