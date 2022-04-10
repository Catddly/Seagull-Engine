#include "StdAfx.h"
#include "RendererVulkan/GUI/TestGUILayer.h"

#include "Render/CommonRenderData.h"
#include "Scene/Mesh/MeshDataArchive.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"

#include "imgui/imgui.h"

// TEMPORARY
#include "glm/gtc/type_ptr.hpp"

namespace SG
{

	void TestGUILayer::OnUpdate(float deltaTime)
	{
		DrawLightPanel();
		DrawStatistics(deltaTime);
	}

	void TestGUILayer::DrawLightPanel()
	{
		ImGui::Begin("Light");
		SSystem()->GetMainScene()->TraversePointLight([](PointLight& pointLight)
			{
				Vector3f position = pointLight.GetPosition();
				float radius = pointLight.GetRadius();
				Vector3f color = pointLight.GetColor();
				ImGui::DragFloat3("Position", glm::value_ptr(position), 0.05f);
				ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f);
				ImGui::ColorEdit3("Color", glm::value_ptr(color));

				pointLight.SetPosition(position);
				pointLight.SetRadius(radius);
				pointLight.SetColor(color);
			});

		// TODO: ui should not directly modify the value of ubo. User can modify it via global settings.
		auto& compositionUbo = GetCompositionUBO();
		ImGui::DragFloat("Gamma", &compositionUbo.gamma, 0.05f, 1.0f, 10.0f);
		ImGui::DragFloat("Exposure", &compositionUbo.exposure, 0.05f, 0.01f, 50.0f);
		ImGui::End();
	}

	void TestGUILayer::DrawStatistics(float deltaTime)
	{
		// update fps
		mElapsedTime += deltaTime;
		if (mElapsedTime >= 0.5f)
		{
			mLastFps = UInt32(1.0f / deltaTime);
			mElapsedTime -= 0.5f;
		}

		Size cullMeshCnt = SSystem()->GetMainScene()->GetNumMesh();
		UInt32 drawCallCnt = MeshDataArchive::GetInstance()->GetNumMeshData();
		auto& statisticData = GetStatisticData();

		ImGui::Begin("Statistics");
		ImGui::Text("Fps: %u", mLastFps);
		ImGui::Separator();
		ImGui::Text("DrawCall: %d", drawCallCnt);
		ImGui::Text("Scene Objects: %d", cullMeshCnt);
		ImGui::Text("Culled Objects: %d", cullMeshCnt - statisticData.cullSceneObjects);
		ImGui::End();
	}

}