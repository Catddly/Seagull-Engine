#include "StdAfx.h"

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
		SG_LOG_DEBUG("Debug Test!");
		SG_LOG_WARN("Warn Test!");
		SG_LOG_ERROR("Error Test!");
		SG_LOG_CRIT("Criticle Test!");
	}

	virtual void OnShutdown() override
	{
		SG_LOG_INFO("User OnExit()");
	}
};

SG::IApp* SG::GetAppInstance()
{
	return New<MyApp>();
}