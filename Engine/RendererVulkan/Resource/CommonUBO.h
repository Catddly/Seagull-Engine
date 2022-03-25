#pragma once

#include "RendererVulkan/Config.h"
#include "Math/MathBasic.h"

namespace SG
{

	struct CameraUBO
	{
		Matrix4f view;
		Matrix4f proj;
		Matrix4f viewProj;
		Vector3f viewPos;
	};

	struct SkyboxUBO
	{
		Matrix4f proj;
		Matrix4f model;
	};

	struct ShadowUBO
	{
		Matrix4f lightSpaceVP;
	};

	// For now, only support one point light and one directional light
	struct LightUBO
	{
		// directional light
		Matrix4f lightSpaceVP;
		Vector3f viewDirection;
		float    pad;
		Vector4f directionalColor;
		// point light
		Vector3f pointLightPos;
		float    pointLightRadius;
		Vector3f pointLightColor;
	};

	struct CompositionUBO
	{
		float gamma;
		float exposure;
	};

	struct PerMeshRenderData
	{
		Matrix4f model = Matrix4f(1.0f);
		Matrix4f inverseTransposeModel = Matrix4f(1.0f);
	};

	struct PerInstanceData
	{
		Vector3f instancePos = Vector3f(0.0f, 0.0f, 0.0f);
		float    instanceScale = 1.0f;

		PerInstanceData() = default;
		PerInstanceData(const Vector3f& insPos, float insScale)
			:instancePos(insPos), instanceScale(insScale)
		{}
	};

	// temporary
	SG_RENDERER_VK_API SG_INLINE LightUBO&  GetLightUBO();
	SG_RENDERER_VK_API SG_INLINE ShadowUBO& GetShadowUBO();
	SG_RENDERER_VK_API SG_INLINE SkyboxUBO& GetSkyboxUBO();
	SG_RENDERER_VK_API SG_INLINE CameraUBO& GetCameraUBO();
	SG_RENDERER_VK_API SG_INLINE CompositionUBO& GetCompositionUBO();
}