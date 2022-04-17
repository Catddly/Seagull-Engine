#include "StdAfx.h"
#include "RendererVulkan/GUI/DockSpaceLayer.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Math/MathBasic.h"
#include "Stl/Utility.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "imgui/imgui.h"

#include "glm/gtc/type_ptr.hpp"

namespace SG
{

	DockSpaceLayer::DockSpaceLayer()
		:ILayer("Dockspace")
	{
		Input::RegisterListener(EListenerPriority::eLevel0, this);
	}

	DockSpaceLayer::~DockSpaceLayer()
	{
		Input::RemoveListener(this);
	}

	void DockSpaceLayer::OnAttach()
	{
		mpViewportTexHandle = VK_RESOURCE()->GetDescriptorSetHandle("ViewportTex");
	}

	void DockSpaceLayer::OnUpdate(float deltaTime)
	{
		DrawDockSpaceBackground();
		DrawMainViewport();
		DrawLightPanel();
		DrawStatistics(deltaTime);
	}

	bool DockSpaceLayer::OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos)
	{
		// just block other events, so that the camera do not get the input update message.
		if (mbViewportCanUpdateMouse)
			return true;
		else
			return false;
	}

	void DockSpaceLayer::DrawDockSpaceBackground()
	{
		static bool sbFullsrceen = true;
		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		if (sbFullsrceen)
		{
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		static bool sbOpen = true;
		ImGui::Begin("_MyDockSpace", &sbOpen, windowFlags);

		ImGui::PopStyleVar();
		if (sbFullsrceen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigDockingWithShift = true;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Setting"))
			{
				if (ImGui::MenuItem("Exit"))
					SG_LOG_DEBUG("Exit!");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}

	void DockSpaceLayer::DrawMainViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

		static bool sbOpen = true;
		static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("MainViewport", &sbOpen, windowFlags);

		mbViewportCanUpdateMouse = ImGui::IsWindowFocused() && ImGui::IsWindowHovered();

		auto viewportSize = ImGui::GetContentRegionAvail();
		if (viewportSize.x != mLastViewportSize.x || viewportSize.y != mLastViewportSize.y) // resize the viewport render target
		{
			mLastViewportSize.x = viewportSize.x;
			mLastViewportSize.y = viewportSize.y;

			mMessageBusMember.PushEvent<Vector2f>("ViewportResizeEvent", mLastViewportSize);
		}

		ImGui::Image(mpViewportTexHandle, viewportSize);
		ImGui::End();

		ImGui::PopStyleVar();
	}

	void DockSpaceLayer::DrawLightPanel()
	{
		bool bShow = true;
		ImGui::ShowDemoWindow(&bShow);

		ImGui::Begin("Light");

		auto lights = SSystem()->GetMainScene()->View<LightTag>();
		for (auto& entity : lights)
		{
			if (entity.HasComponent<PointLightComponent>())
			{
				ImGui::Separator();
				auto [tag, trans, light] = entity.GetComponent<TagComponent, TransformComponent, PointLightComponent>();

				tag.bDirty |= ImGui::DragFloat3("Point Light Position", glm::value_ptr(trans.position), 0.05f);
				tag.bDirty |= ImGui::DragFloat("Point Light Radius", &light.radius, 0.1f, 0.0f);
				tag.bDirty |= ImGui::ColorEdit3("Point Light Color", glm::value_ptr(light.color));
			}
			else if (entity.HasComponent<DirectionalLightComponent>())
			{
				ImGui::Separator();
				auto [tag, trans, light] = entity.GetComponent<TagComponent, TransformComponent, DirectionalLightComponent>();

				tag.bDirty |= ImGui::DragFloat3("Directional Light Position", glm::value_ptr(trans.position), 0.05f);
				tag.bDirty |= ImGui::DragFloat3("Directional Light Rotation", glm::value_ptr(trans.rotation), 0.05f);
				tag.bDirty |= ImGui::ColorEdit3("Directional Light Color", glm::value_ptr(light.color));
			}
		}

		ImGui::Separator();
		// TODO: ui should not directly modify the value of ubo. User can modify it via global settings.
		auto& compositionUbo = GetCompositionUBO();
		ImGui::DragFloat("Gamma", &compositionUbo.gamma, 0.05f, 1.0f, 10.0f);
		ImGui::DragFloat("Exposure", &compositionUbo.exposure, 0.05f, 0.01f, 50.0f);

		ImGui::End();
	}

	void DockSpaceLayer::DrawStatistics(float deltaTime)
	{
		mElapsedTime += deltaTime;
		if (mElapsedTime >= 0.5f)
		{
			mLastFps = UInt32(1.0f / deltaTime);
			mElapsedTime -= 0.5f;
		}

		Size cullMeshCnt = SSystem()->GetMainScene()->GetMeshEntityCount();
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