#include "StdAfx.h"

#include <iostream>

class MyApp : public SG::IApp
{
public:
	virtual void OnInit() override
	{
		SG_LOG_INFO("User OnInit()");
		SG_LOG_INFO("User OnUpdate()");
		SG_LOG_DEBUG("Debug Test!");
		SG_LOG_WARN("Warn Test!");
		SG_LOG_ERROR("Error Test!");
		SG_LOG_CRIT("Criticle Test!");

		SG::vector<SG::UInt32> vec = { 8, 9, 5, 4, 2 };
		SG_LOG_ITERABLE(SG::ELogLevel::eLog_Level_Debug, vec.begin(), vec.end());
		SG_LOG_DEBUG("%d", vec.at(2));

		MathTest();
	}

	virtual void OnUpdate() override
	{
	}

	virtual void OnShutdown() override
	{
		SG_LOG_INFO("User OnExit()");
	}
private:
	void MathTest()
	{
		using namespace SG;
		Matrix4f mat4f = Matrix4f::Identity();
		Matrix3f mat3f = Matrix3f::Identity();
		Matrix3i mat3i = Matrix3i::Identity();
		Matrix4i mat4i = Matrix4i::Identity();

		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, mat3f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, mat4f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, mat3i);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, mat4i);

		Vector3f vec3f = Vector3f::Identity();
		Vector4f vec4f = Vector4f::Identity();
		Vector3i vec3i = Vector3i::Identity();
		Vector4i vec4i = Vector4i::Identity();

		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec3f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec4f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec3i);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec4i);
	}
};

SG::IApp* SG::GetAppInstance()
{
	return new MyApp;
}