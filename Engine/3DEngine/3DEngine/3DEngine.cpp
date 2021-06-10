#include "StdAfx.h"
#include "3DEngine.h"

#include "Common/System/ISystem.h"
#include "Common/System/ILog.h"

namespace SG
{

	void C3DEngine::OnInit()
	{
		SG_LOG_INFO("Engine OnInit()");
	}

	void C3DEngine::OnUpdate()
	{
		//SG_LOG_INFO("Engine OnUpdate()");
	}

	void C3DEngine::OnShutdown()
	{
		SG_LOG_INFO("Engine OnExit()");
	}

}