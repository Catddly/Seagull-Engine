#include "StdAfx.h"
#include "Engine.h"

#include "Common/System/ISystem.h"
#include "Log/Log/Log.h"

namespace SG
{

	void CEngine::OnInit()
	{
		SG_LOG_INFO("Engine OnInit()");
	}

	void CEngine::OnUpdate()
	{
		SG_LOG_INFO("Engine OnUpdate()");
	}

	void CEngine::OnShutdown()
	{
		SG_LOG_INFO("Engine OnExit()");
	}

	void CEngine::GetMainThreadId() const
	{
		SG_LOG_INFO("Engine GetMainThreadId()");
	}

}