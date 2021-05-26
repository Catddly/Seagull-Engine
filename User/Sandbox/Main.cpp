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

		int arr[] = { 2, 6, 4, 8, 6 };
		SpanTest(arr);
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
		SG_LOG_ITERABLE(ELogLevel::eLOG_LEVEL_DEBUG, vec2);
		SG_LOG_ITERABLE_R(ELogLevel::eLOG_LEVEL_DEBUG, vec2);

		//SG_LOG_DEBUG("Dim of arr is %d", rank_v<int[2][3]>);
		//SG_LOG_DEBUG("Dim of arr is %d", rank_v<int[2][3][2]>);
		//SG_LOG_DEBUG("Dim of arr is %d", rank_v<int[2]>);
		//SG_LOG_DEBUG("Dim of arr is %d", rank_v<int>);

		//SG_LOG_DEBUG("Size of arr is %d", size_of_array<int[2][4]>::value);
		//SG_LOG_DEBUG("Size of arr is %d", size_of_array<double[2][4][2]>::value);
		//SG_LOG_DEBUG("Size of arr is %d", size_of_array<char[4]>::value);
		//SG_LOG_DEBUG("Size of arr is %d", size_of_array<int>::value);
	}

	void SpanTest(SG::span<int> s)
	{
		using namespace SG;
		SG_LOG_ITERATOR(ELogLevel::eLOG_LEVEL_DEBUG, s.begin(), s.end());
		SG_LOG_ITERATOR_R(ELogLevel::eLOG_LEVEL_DEBUG, s.begin(), s.end());
		auto view = s.subspan(2, 2);
		SG_LOG_ITERATOR(ELogLevel::eLOG_LEVEL_DEBUG, view.begin(), view.end());
		auto viewF = s.first(3);
		auto viewL = s.last(3);
		SG_LOG_ITERATOR(ELogLevel::eLOG_LEVEL_DEBUG, viewF.begin(), viewF.end());
		SG_LOG_ITERATOR(ELogLevel::eLOG_LEVEL_DEBUG, viewL.begin(), viewL.end());
	}
};

SG::IApp* SG::GetAppInstance()
{
	return New<MyApp>();
}