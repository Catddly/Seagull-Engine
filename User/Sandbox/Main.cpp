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

		StringTest();
	}

	virtual void OnShutdown() override
	{
		SG_LOG_INFO("User OnExit()");
	}
private:
	void VectorTest()
	{
		using namespace SG;
		SG_LOG_DEBUG("--------------------------VectorTest()--------------------------");
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
		//vec2 = move(vec);
		vec2.push_back(3);
		vec2.push_back(5);
		SG::swap(vec, vec2);

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
		SG_LOG_DEBUG("--------------------------VectorTest()--------------------------");
	}

	void SpanTest(SG::span<int> s)
	{
		using namespace SG;
		SG_LOG_DEBUG("---------------------------SpanTest()---------------------------");
		SG_LOG_ITERATOR(ELogLevel::eLOG_LEVEL_DEBUG, s.begin(), s.end());
		SG_LOG_ITERATOR_R(ELogLevel::eLOG_LEVEL_DEBUG, s.begin(), s.end());
		auto view = s.subspan(2, 2);
		SG_LOG_ITERATOR(ELogLevel::eLOG_LEVEL_DEBUG, view.begin(), view.end());
		auto viewF = s.first(3);
		auto viewL = s.last(3);
		SG_LOG_ITERATOR(ELogLevel::eLOG_LEVEL_DEBUG, viewF.begin(), viewF.end());
		SG_LOG_ITERATOR(ELogLevel::eLOG_LEVEL_DEBUG, viewL.begin(), viewL.end());
		SG_LOG_DEBUG("---------------------------SpanTest()---------------------------");
	}

	void StringTest()
	{
		using namespace SG;
		SG_LOG_DEBUG("--------------------------StringTest()--------------------------");
		string str = "Hello, ";
		SG_LOG_DEBUG("%s", str.c_str());
		str += "World!";
		SG_LOG_DEBUG("%s", str.c_str());
		SG_LOG_ITERABLE(ELogLevel::eLOG_LEVEL_DEBUG, str);
		str.append(" This is a long long long long long string!");
		SG_LOG_DEBUG("%s", str.c_str());
		SG_LOG_DEBUG("%s", str.substr(0, 13).c_str());
		string str2("short string!");
		swap(str, str2);
		SG_LOG_DEBUG("%s", str.c_str());
		str2.assign("Hello!");
		SG_LOG_DEBUG("%s", str2.c_str());
		string str3(str2);
		str3 = str2 + " Fuck!";
		string str4(str);
		SG_LOG_DEBUG("str is %s", str.c_str());
		SG_LOG_DEBUG("str2 is %s", str2.c_str());
		SG_LOG_DEBUG("str3 is %s", str3.c_str());
		SG_LOG_DEBUG("str4 is %s", str4.c_str());

		SG_LOG_DEBUG("Is the same? %d", str == str2);
		SG_LOG_DEBUG("Is the same? %d", str4 == str);
		SG_LOG_DEBUG("Is str3 > str2? %d", str3 > str2);
		SG_LOG_DEBUG("Is str > str4? %d", str < str4);

		SG_LOG_DEBUG("First 'l' of str2: %d", str2.find_first_of('l', 0));
		SG_LOG_DEBUG("First 'l' of str2: %d", str2.find_first_of('lo', 0));
		SG_LOG_DEBUG("Find 'F' of str3: %d", str3.find('F'));
		str3.push_back('F');
		SG_LOG_DEBUG("%s", str3.c_str());
		str3.append(str);
		SG_LOG_DEBUG("%s", str3.c_str());
		SG_LOG_DEBUG("--------------------------StringTest()--------------------------");
	}
};

SG::IApp* SG::GetAppInstance()
{
	return New<MyApp>();
}