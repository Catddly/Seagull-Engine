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

		VectorTest();
	}

	virtual void OnShutdown() override
	{
		SG_LOG_INFO("User OnExit()");
	}
private:
	void VectorTest()
	{
		using namespace SG;
		vector<UInt32> vec;
		SG_LOG_DEBUG("vector empty: %d", vec.empty());
		vec.push_back(6);
		vec.push_back(9);
		vec.push_back(1);
		vec.insert(1, 2);
		vec.emplace_back(14);
		vec.emplace_back(12);
		SG_LOG_ITERABLE(ELogLevel::eLOG_LEVEL_DEBUG, vec);
		SG_LOG_DEBUG("size of vector: %d", vec.size());
		SG_LOG_DEBUG("capacity of vector: %d", vec.capacity());
		SG_LOG_DEBUG("%d", vec.pop_back());
		SG_LOG_DEBUG("4th of vector: %d", vec[4]);
		SG_LOG_DEBUG("size of vector: %d", vec.size());
		vector<UInt32> vec2;
		vec2 = move(vec);
		SG_LOG_ITERABLE(ELogLevel::eLOG_LEVEL_INFO, vec2);
		SG_LOG_ITERABLE_R(ELogLevel::eLOG_LEVEL_INFO, vec2);
	}
};

SG::IApp* SG::GetAppInstance()
{
	return New<MyApp>();
}