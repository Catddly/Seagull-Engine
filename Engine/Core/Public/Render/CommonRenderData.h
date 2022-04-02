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
		Vector4f frustum[6];
		Vector3f viewPos;
		float    pad;
		UInt32   numObjects;
		UInt32   numDrawCalls;
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
		Vector2f MR = { 0.2f, 0.85f };
		Int32    baseIndex = -1; // base index of the instance
		UInt32   meshId;
	};

	struct CullingOutputData
	{
		Vector3f albedo;
		float    pad;
	};

	struct PerInstanceData
	{
		UInt32 objectId;

		PerInstanceData() = default;
		PerInstanceData(UInt32 id)
			:objectId(id)
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