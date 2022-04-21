#include "StdAfx.h"
#include "RendererVulkan/GUI/DockSpaceLayer.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Math/MathBasic.h"
#include "Stl/Utility.h"
#include "Scene/Components.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/GUI/Utility.h"

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
		DrawSceneHirerchy();
		DrawMainViewport();
		DrawSelectedEntityProperty();
		DrawStatistics(deltaTime);
		DrawSettingPanel();
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
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
					SSystem()->Terminate();
				ImGui::EndMenu();
			}

			//if (ImGui::BeginMenu("Profile"))
			//{
			//	if (ImGui::MenuItem("Pipeline Statistics"))
			//		mbShowStatisticsDetail = !mbShowStatisticsDetail;
			//	ImGui::EndMenu();
			//}

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

			SSystem()->GetMainScene()->GetMainCamera()->SetPerspective(60.0f, mLastViewportSize.x / mLastViewportSize.y);
			//mMessageBusMember.PushEvent<Vector2f>("ViewportResizeEvent", mLastViewportSize);
		}

		ImGui::Image(mpViewportTexHandle, viewportSize);
		ImGui::End();

		ImGui::PopStyleVar();
	}

	void DockSpaceLayer::DrawSettingPanel()
	{
		ImGui::Begin("Global Settings");

		ImGui::Separator();

		// TODO: ui should not directly modify the value of ubo. User can modify it via global settings.
		auto& compositionUbo = GetCompositionUBO();
		DrawGUIDragFloat("Gamma", compositionUbo.gamma, 2.2f, 0.05f, 1.0f, 10.0f);
		DrawGUIDragFloat("Exposure", compositionUbo.exposure, 1.0f, 0.05f, 0.01f, 50.0f);

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
		ImGui::Text("DrawCall: %d", drawCallCnt);
		ImGui::Text("Scene Objects: %d", cullMeshCnt);
		ImGui::Text("Culled Objects: %d", cullMeshCnt - statisticData.cullSceneObjects);
		ImGui::Checkbox("Show Detail", &mbShowStatisticsDetail);
		if (mbShowStatisticsDetail)
		{
			double total = statisticData.gpuRenderPassTime[0] + statisticData.gpuRenderPassTime[1] + statisticData.gpuRenderPassTime[2];
			ImGui::Separator();
			ImGui::Text("Fps GPU: %u", UInt32(1000.0 / total));
			ImGui::Text("Shadow Pass: %.4lf ms", statisticData.gpuRenderPassTime[0]);
			ImGui::Text("Draw Pass: %.4lf ms", statisticData.gpuRenderPassTime[1]);
			ImGui::Text("UI Pass: %.4lf ms", statisticData.gpuRenderPassTime[2]);
			ImGui::Text("GPU Total: %.4lf ms", total);
			ImGui::Separator();
			ImGui::Text("Input assembly vertices: %d", statisticData.pipelineStatistics[0]);
			ImGui::Text("Input assembly primitives: %d", statisticData.pipelineStatistics[1]);
			ImGui::Text("Vertex shader invocations: %d", statisticData.pipelineStatistics[2]);
			ImGui::Text("Clipping invocations: %d", statisticData.pipelineStatistics[3]);
			ImGui::Text("Clipping primitives: %d", statisticData.pipelineStatistics[4]);
			ImGui::Text("Fragment shader invocations: %d", statisticData.pipelineStatistics[5]);
			//ImGui::Text("Culling Reset Compute shader invocations: %d", statisticData.pipelineStatistics[6]);
			//ImGui::Text("Culling Compute shader invocations: %d", statisticData.pipelineStatistics[7]);
		}
		ImGui::End();
	}

	void DockSpaceLayer::DrawSceneHirerchy()
	{
		ImGui::Begin("Scene");

		auto pScene = SSystem()->GetMainScene();
		// draw entity tree
		pScene->TraverseEntity([this](auto& entity) 
			{
				auto& tag = entity.GetComponent<TagComponent>();
				bool bOpened = ImGui::TreeNodeEx(tag.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);

				if (ImGui::IsItemClicked())
					mSelectedEntity = entity;

				if (bOpened)
					ImGui::TreePop();
			});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			mSelectedEntity = { };

		ImGui::End();
	}

	void DockSpaceLayer::DrawSelectedEntityProperty()
	{
		ImGui::Begin("Property");

		if (mSelectedEntity.IsValid())
			DrawEntityProperty(mSelectedEntity);

		ImGui::End();
	}

}