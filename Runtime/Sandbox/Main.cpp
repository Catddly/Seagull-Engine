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

		SG::vector<SG::UInt32> vec = { 8, 9, 5, 4, 2 };
		SG_LOG_ITERABLE(SG::ELogLevel::eLOG_LEVEL_DEBUG, vec.begin(), vec.end());
		SG_LOG_DEBUG("%d", vec.at(2));
	}

	virtual void OnShutdown() override
	{
		SG_LOG_INFO("User OnExit()");
	}
private:
};

SG::IApp* SG::GetAppInstance()
{
	return New<MyApp>();
}