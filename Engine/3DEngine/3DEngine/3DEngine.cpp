#include "StdAfx.h"
#include "3DEngine.h"

#include "System/System.h"
#include "System/ILogger.h"

namespace SG
{

	void C3DEngine::OnInit()
	{
		SG_LOG_INFO("Engine OnInit()");
	}

	void C3DEngine::OnUpdate(float deltaTime)
	{
		//SG_LOG_INFO("Engine OnUpdate()");
	}

	void C3DEngine::OnShutdown()
	{
		SG_LOG_INFO("Engine OnExit()");
	}

}