#include "StdAfx.h"
#include "RendererVulkan/Resource/CommonUBO.h"

namespace SG
{

	static LightUBO  gLightUbo = {};
	static ShadowUBO gShadowUbo = {};
	static CameraUBO gCameraUbo = {};

	LightUBO& GetLightUBO()
	{
		return gLightUbo;
	}

	ShadowUBO& GetShadowUBO()
	{
		return gShadowUbo;
	}

	CameraUBO& GetCameraUBO()
	{
		return gCameraUbo;
	}

}