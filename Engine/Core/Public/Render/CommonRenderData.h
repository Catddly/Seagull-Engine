#pragma once

#include "RendererVulkan/Config.h"
#include "Math/MathBasic.h"

namespace SG
{

	struct Vertex
	{
		Vector3f position;
		Vector3f normal;
	};

	struct CameraUBO
	{
		Matrix4f view;
		Matrix4f proj;
		Matrix4f viewProj;
		Vector3f viewPos;
	};

	struct GPUCullUBO
	{
		Vector3f viewPos;
		float    zNear;
		Vector3f rightVec;
		float    zFar;
		Vector3f frontVec;
		float    fovY;
		Vector3f upVec;
		float    aspectRatio;
		UInt32   numObjects;
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

	struct ObjcetRenderData
	{
		Matrix4f model = Matrix4f(1.0f);
		Matrix4f inverseTransposeModel = Matrix4f(1.0f);
		Vector4f MRXX = { 0.2f, 0.85f, 0.0f, 0.0f }; // metallic, roughness, undefined, undefined
	};

	struct CullingOutputData
	{
		Vector3f albedo;
		float    pad;
	};

	struct PerInstanceData
	{
		Vector3f instancePos = Vector3f(0.0f, 0.0f, 0.0f);
		float    instanceScale = 1.0f;
		float    instanceMetallic = 0.0f;
		float    instanceRoughness = 0.0f;
		UInt32   objectId;

		PerInstanceData() = default;
		PerInstanceData(const Vector3f& insPos, float insScale, float metallic, float roughness, UInt32 id)
			:instancePos(insPos), instanceScale(insScale), instanceMetallic(metallic), instanceRoughness(roughness), objectId(id)
		{}
	};

	// temporary
	SG_RENDERER_VK_API SG_INLINE LightUBO&  GetLightUBO();
	SG_RENDERER_VK_API SG_INLINE ShadowUBO& GetShadowUBO();
	SG_RENDERER_VK_API SG_INLINE SkyboxUBO& GetSkyboxUBO();
	SG_RENDERER_VK_API SG_INLINE CameraUBO& GetCameraUBO();
	SG_RENDERER_VK_API SG_INLINE CompositionUBO& GetCompositionUBO();
	SG_RENDERER_VK_API SG_INLINE GPUCullUBO& GetGPUCullUBO();
}