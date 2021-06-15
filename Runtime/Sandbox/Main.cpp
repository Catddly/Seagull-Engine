#include "StdAfx.h"

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

		//MathTest();
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

		Vector2f vec2f = Vector2f::Identity();
		Vector3f vec3f = Vector3f::Identity();
		Vector4f vec4f = Vector4f::Identity();
		Vector2i vec2i = Vector2i::Identity();
		Vector3i vec3i = Vector3i::Identity();
		Vector4i vec4i = Vector4i::Identity();

		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec2f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec3f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec4f);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec2i);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec3i);
		SG_LOG_MATH(ELogLevel::eLog_Level_Debug, vec4i);
	}
};

SG::IApp* SG::GetAppInstance()
{
	return new MyApp;
}