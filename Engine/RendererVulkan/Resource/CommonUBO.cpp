#include "StdAfx.h"
#include "RendererVulkan/Resource/CommonUBO.h"

namespace SG
{

	static LightUBO  gLightUbo = {};
	static SkyboxUBO gSkyboxUbo = {};
	static ShadowUBO gShadowUbo = {};
	static CameraUBO gCameraUbo = {};
	static CompositionUBO gCompositionUbo = {};

	LightUBO& GetLightUBO()
	{
		return gLightUbo;
	}

	ShadowUBO& GetShadowUBO()
	{
		return gShadowUbo;
	}

	SkyboxUBO& GetSkyboxUBO()
	{
		return gSkyboxUbo;
	}

	CameraUBO& GetCameraUBO()
	{
		return gCameraUbo;
	}

	CompositionUBO& GetCompositionUBO()
	{
		return gCompositionUbo;
	}

}