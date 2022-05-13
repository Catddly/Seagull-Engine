#pragma once

#include "RendererVulkan/Config.h"
#include "Math/MathBasic.h"

#include "eastl/array.h"

namespace SG
{

	struct Vertex
	{
		Vector3f position;
		Vector3f normal;
		Vector2f uv0;
		Vector3f tangent;
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
		// transform
		Matrix4f model = Matrix4f(1.0f);
		Matrix4f inverseTransposeModel = Matrix4f(1.0f);
		// material
		Vector2f MR = { 0.2f, 0.85f }; // metallic and roughness
		Int32    instanceOffset; // instanceId, if it have no instance, id = -1.
		Int32    meshId;
		Vector3f albedo;
		UInt32   texFlag; //! Used to determined which texture map should be use.
	};

	struct CullingOutputData
	{
		Vector3f albedo;
		float    pad;
	};

	struct InstanceOutputData
	{
		Int32  testIndex = -1; //! if testIndex is -1, it means this instance is invalid, should not be drawn on screen.
		UInt32 objectId = 0;
	};

	struct PerInstanceData
	{
		UInt32 objectId;

		PerInstanceData() = default;
		PerInstanceData(UInt32 id)
			:objectId(id)
		{}
	};

	struct StatisticData
	{
		UInt32 culledSceneObjects = 0;
		eastl::array<UInt64, 8> pipelineStatistics = {};
		eastl::array<double, 3> gpuRenderPassTime;
	};

	// temporary
	SG_RENDERER_VK_API LightUBO&  GetLightUBO();
	SG_RENDERER_VK_API ShadowUBO& GetShadowUBO();
	SG_RENDERER_VK_API SkyboxUBO& GetSkyboxUBO();
	SG_RENDERER_VK_API CameraUBO& GetCameraUBO();
	SG_RENDERER_VK_API CompositionUBO& GetCompositionUBO();
	SG_RENDERER_VK_API GPUCullUBO& GetGPUCullUBO();

	SG_RENDERER_VK_API StatisticData& GetStatisticData();

}