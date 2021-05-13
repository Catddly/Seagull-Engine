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
		vec.push_back(3);
		vec.push_back(14);
		for (auto beg = vec.begin(); beg != vec.end(); beg++)
			SG_LOG_DEBUG("%d", *beg);
		SG_LOG_DEBUG("size of vector: %d", vec.size());
		SG_LOG_DEBUG("capacity of vector: %d", vec.capacity());
		SG_LOG_DEBUG("%d", vec.pop_back());
		SG_LOG_DEBUG("size of vector: %d", vec.size());
		SG_LOG_DEBUG("capacity of vector: %d", vec.capacity());
		vec.push_back(34);
		vec.push_back(73);
		vec.push_back(95);
		vec.push_back(915);
		vec.push_back(142);
		SG_LOG_DEBUG("4th of vector: %d", vec[4]);
		SG_LOG_DEBUG("size of vector: %d", vec.size());
		SG_LOG_DEBUG("capacity of vector: %d", vec.capacity());
		SG_LOG_DEBUG("vector empty: %d", vec.empty());
	}
};

SG::IApp* SG::GetAppInstance()
{
	return New<MyApp>();
}