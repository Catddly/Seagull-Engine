#include "StdAfx.h"
#include "RendererVulkan/GUI/DockSpaceLayer.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Math/MathBasic.h"
#include "Stl/Utility.h"
#include "Scene/Components.h"
#include "Archive/Serialization.h"

#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/GUI/Utility.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "glm/gtc/type_ptr.hpp"

namespace SG
{

	namespace
	{
		static ImGuiTextFilter gTextFilter;
	}

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
		SG_PROFILE_FUNCTION();

		mViewportTexHandle = VK_RESOURCE()->GetDescriptorSetHandle("ViewportTex").ReadOnly();
	}

	void DockSpaceLayer::OnDetach()
	{
		SG_PROFILE_FUNCTION();
	}

	void DockSpaceLayer::OnUpdate(float deltaTime)
	{
		SG_PROFILE_FUNCTION();

		DrawDockSpaceBackground();
		DrawSceneHirerchy();
		DrawMainViewport();

		DrawSelectedEntityProperty();
		DrawStatistics(deltaTime);
		DrawSettingPanel();

		static float showProgressWindowTime = 0.0f;
		if (mbShowSaveSceneProgressBar)
		{
			// to avoid the duration user used in the file dialog
			showProgressWindowTime += deltaTime <= 0.75f ? deltaTime : 0.0f;

			if (showProgressWindowTime >= 2.5f)
			{
				showProgressWindowTime = 0.0f;
				mbShowSaveSceneProgressBar = false;
			}
			else
				DrawSaveSceneProgressBar();
		}
	}

	bool DockSpaceLayer::OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState)
	{
		SG_PROFILE_FUNCTION();

		if (mbTriggerOpen && !Input::IsKeyPressed(KeyCode_LeftControl) || !Input::IsKeyPressed(KeyCode_O))
			mbTriggerOpen = false;

		if (mbTriggerSave && !Input::IsKeyPressed(KeyCode_LeftControl) || !Input::IsKeyPressed(KeyCode_S))
			mbTriggerSave = false;

		if (mbTriggerSaveAs && !Input::IsKeyPressed(KeyCode_LeftControl) || !Input::IsKeyPressed(KeyCode_LeftShift) || !Input::IsKeyPressed(KeyCode_S))
			mbTriggerSaveAs = false;

		if (mbTriggerNew && !Input::IsKeyPressed(KeyCode_LeftControl) || !Input::IsKeyPressed(KeyCode_N))
			mbTriggerNew = false;

		if (mbViewportOnFocused)
			return true;
		else
		{
			if (!mbTriggerSaveAs && Input::IsKeyPressed(KeyCode_LeftControl) && Input::IsKeyPressed(KeyCode_LeftShift) && Input::IsKeyPressed(KeyCode_S))
			{
				SaveAsScene();
				mbTriggerSaveAs = true;
			}
			else if (!mbTriggerOpen && Input::IsKeyPressed(KeyCode_LeftControl) && Input::IsKeyPressed(KeyCode_O))
			{
				OpenScene();
				mbTriggerOpen = true;
			}
			else if (!mbTriggerSaveAs && !mbTriggerSave && Input::IsKeyPressed(KeyCode_LeftControl) && Input::IsKeyPressed(KeyCode_S))
			{
				SaveScene();
				mbTriggerSave = true;
			}
			else if (!mbTriggerNew && Input::IsKeyPressed(KeyCode_LeftControl) && Input::IsKeyPressed(KeyCode_N))
			{
				NewScene();
				mbTriggerNew = true;
			}
		}

		return false;
	}

	bool DockSpaceLayer::OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos)
	{
		SG_PROFILE_FUNCTION();

		// just block other events, so that the camera do not get the input update message.
		if (mbViewportOnFocused && mbViewportOnHovered)
			return true;
		else
			return false;
	}

	void DockSpaceLayer::DrawDockSpaceBackground()
	{
		SG_PROFILE_FUNCTION();

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
			ImGuiID dockspaceId = ImGui::GetID("_MyDockSpace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene", "CTRL+N"))
					NewScene();

				ImGui::Separator();
				if (ImGui::MenuItem("Open Scene", "CTRL+O"))
					OpenScene();
				
				ImGui::Separator();
				if (ImGui::MenuItem("Save Scene", "CTRL+S"))
					SaveScene();
				
				if (ImGui::MenuItem("Save Scene As", "CTRL+SHIFT+S"))
					SaveAsScene();
				
				ImGui::Separator();
				if (ImGui::MenuItem("Exit", "ESC"))
					SSystem()->Terminate();
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Profile"))
			{
				if (ImGui::MenuItem("Open Tracy Profiler"))
				{
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		ImGui::End();
	}

	void DockSpaceLayer::DrawMainViewport()
	{
		SG_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

		static bool sbOpen = true;
		static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("MainViewport", &sbOpen, windowFlags);
		auto windowPos = ImGui::GetWindowPos();
		mViewportPos.x = windowPos.x;
		mViewportPos.y = windowPos.y;

		mbViewportOnFocused = ImGui::IsWindowFocused();
		mbViewportOnHovered = ImGui::IsWindowHovered();

		auto viewportSize = ImGui::GetContentRegionAvail();
		if (viewportSize.x != mLastViewportSize.x || viewportSize.y != mLastViewportSize.y) // resize the viewport render target
		{
			mLastViewportSize.x = viewportSize.x;
			mLastViewportSize.y = viewportSize.y;

			auto& cam = SSystem()->GetMainScene()->GetMainCamera();
			cam.GetComponent<CameraComponent>().pCamera->SetPerspective(60.0f, mLastViewportSize.x / mLastViewportSize.y);
			//mMessageBusMember.PushEvent<Vector2f>("ViewportResizeEvent", mLastViewportSize);
		}

		ImGui::Image(mViewportTexHandle.GetData(), viewportSize);
		ImGui::End();

		ImGui::PopStyleVar();
	}

	void DockSpaceLayer::DrawSettingPanel()
	{
		SG_PROFILE_FUNCTION();

		//bool bShow = true;
		//ImGui::ShowDemoWindow(&bShow);

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
		SG_PROFILE_FUNCTION();

		mElapsedTime += deltaTime;
		mFrameCounter++;
		if (mElapsedTime >= 1.0f)
		{
			mLastFps = static_cast<UInt32>((float)mFrameCounter / mElapsedTime);
			mElapsedTime = 0.0f;
			mFrameCounter = 1;
		}

		Size meshCnt = SSystem()->GetMainScene()->GetMeshEntityCount();
		UInt32 drawCallCnt = MeshDataArchive::GetInstance()->GetNumMeshData();
		auto& statisticData = GetStatisticData();

		ImGui::Begin("Statistics");
		ImGui::Text("Avg Fps: %u", mLastFps);
		ImGui::Text("DrawCall: %d", drawCallCnt);
		ImGui::Text("Scene Objects: %d", meshCnt);
		ImGui::Text("Culled Objects: %d", statisticData.culledSceneObjects);
		if (ImGui::Checkbox("Show Detail", &mbShowStatisticsDetail))
			mMessageBusMember.PushEvent<bool>("StatisticsShowDetailChanged", mbShowStatisticsDetail);
		
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
		SG_PROFILE_FUNCTION();

		//bool bShow = true;
		//ImGui::ShowDemoWindow(&bShow);

		ImGui::Begin("Scene");

		ImGui::Text("Filter");
		ImGui::SameLine();
		gTextFilter.Draw("##Filter", ImGui::GetContentRegionAvail().x);

		auto pScene = SSystem()->GetMainScene();
		auto* pRootNode = pScene->GetTreeRepresentation();

		// draw entity tree
		for (auto* pChild : pRootNode->pChilds)
			DrawSceneTreeNode(pChild, false);

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			mSelectedEntity = { };

		ImGui::End();
	}

	void DockSpaceLayer::DrawSceneTreeNode(Scene::TreeNode* pNode, bool bPass)
	{
		SG_PROFILE_FUNCTION();

		// recursive basis
		//if (!pNode->pEntity)
		//	return;

		// draw entity tree node
		auto& tag = pNode->pEntity->GetComponent<TagComponent>();
		bool bHasChildPass = false;
		if (!bPass)
		{
			bPass = gTextFilter.PassFilter(tag.name.c_str());
			if (!bPass)
			{
				for (auto* pChild : pNode->pChilds)
				{
					bHasChildPass = TestNodePassFilter(pChild);
					if (bHasChildPass)
						break;
				}
			}
		}

		if (bPass || bHasChildPass) // it is a leaf node
		{
			bool bOpened = ImGui::TreeNodeEx(tag.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);

			if (ImGui::IsItemClicked())
				mSelectedEntity = *pNode->pEntity;

			if (bOpened)
			{
				for (auto* pChild : pNode->pChilds)
					DrawSceneTreeNode(pChild, bPass);
				ImGui::TreePop();
			}
		}
	}

	bool DockSpaceLayer::TestNodePassFilter(Scene::TreeNode* pNode)
	{
		SG_PROFILE_FUNCTION();

		if (pNode->pChilds.empty())
			return gTextFilter.PassFilter(pNode->pEntity->GetComponent<TagComponent>().name.c_str());

		for (auto& pChild : pNode->pChilds)
		{
			if (TestNodePassFilter(pChild))
				return true;
		}
		return false;
	}

	void DockSpaceLayer::DrawSelectedEntityProperty()
	{
		SG_PROFILE_FUNCTION();

		ImGui::Begin("Property");

		if (mSelectedEntity.IsValid())
			DrawEntityProperty(mSelectedEntity);

		ImGui::End();
	}

	void DockSpaceLayer::DrawSaveSceneProgressBar()
	{
		SG_PROFILE_FUNCTION();

		enum ECORNER { TOP_LEFT = 0, TOP_RIGHT = 1, BOTTOM_LEFT = 2, BOTTOM_RIGHT = 3 };

		static int corner = TOP_LEFT;
		ImGuiIO& io = ImGui::GetIO();

		const float PAD = 10.0f;
		ImVec2 pos = { mViewportPos.x, mViewportPos.y };
		ImVec2 size = { mLastViewportSize.x, mLastViewportSize.y };
		ImVec2 windowPos, windowPosPivot;
		windowPos.x = (corner & 1) ? (pos.x + size.x - PAD) : (pos.x + PAD);
		windowPos.y = (corner & 2) ? (pos.y + size.y - PAD) : (pos.y + PAD);
		windowPosPivot.x = (corner & 1) ? 1.0f : 0.0f;
		windowPosPivot.y = (corner & 2) ? 1.0f : 0.0f;

		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

		// show progress bar window
		if (ImGui::Begin("Save Scene Progress Bar", &mbShowSaveSceneProgressBar, flags))
		{
			//ImGui::Text("Saving Scene...");
			// Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
			// or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
			//ImGui::ProgressBar(0.65f, ImVec2(0.0f, 0.0f));
			ImGui::Text("Scene successfully saved to Scene/%s.", mSavedSceneName.c_str());
		}
		ImGui::End();
	}

	void DockSpaceLayer::NewScene()
	{
		MeshDataArchive::GetInstance()->Reset();

		auto pNewScene = SSystem()->NewScene();
		pNewScene->OnSceneLoad();

		MeshDataArchive::GetInstance()->LogDebugInfo();

		pNewScene->GetMainCamera().GetComponent<CameraComponent>().pCamera->SetPerspective(60.0f, mLastViewportSize.x / mLastViewportSize.y);

		OnSceneRebuild(pNewScene);
	}

	void DockSpaceLayer::OpenScene()
	{
		string path = FileSystem::OpenFileDialog(OperatingSystem::GetMainWindow(), "SG Scene (*.scene)\0*.scene\0");
		if (path.empty())
			return;

		MeshDataArchive::GetInstance()->Reset();

		Size pos = path.find_last_of("\\\\") + 1;
		string sceneName = path.substr(pos, path.size() - pos);
		auto pNewScene = SSystem()->NewScene();
		Deserializer::Deserialize(pNewScene, sceneName.c_str());

		MeshDataArchive::GetInstance()->LogDebugInfo();

		pNewScene->GetMainCamera().GetComponent<CameraComponent>().pCamera->SetPerspective(60.0f, mLastViewportSize.x / mLastViewportSize.y);

		OnSceneRebuild(pNewScene);
	}

	void DockSpaceLayer::SaveScene()
	{
		Serializer::Serialize(SSystem()->GetMainScene(), "default.scene");
		mSavedSceneName = "default.scene";
		mbShowSaveSceneProgressBar = true;

		Input::ForceReleaseAllEvent(); // to avoid shourt cut key status
	}

	void DockSpaceLayer::SaveAsScene()
	{
		string path = FileSystem::SaveFileDialog(OperatingSystem::GetMainWindow(), "SG Scene (*.scene)\0*.scene\0");
		if (path.empty())
			return;

		if (path.find('.') == string::npos)
			path += ".scene";
		Size pos = path.find_last_of("\\\\") + 1;
		string sceneName = path.substr(pos, path.size() - pos);
		Serializer::Serialize(SSystem()->GetMainScene(), sceneName.c_str());
		mSavedSceneName = sceneName;
		mbShowSaveSceneProgressBar = true;

		Input::ForceReleaseAllEvent(); // to avoid short cut key status
	}

	void DockSpaceLayer::OnSceneRebuild(RefPtr<Scene> pNewScene)
	{
		// rebuild scene render data
		auto pRenderDataBuilder = SSystem()->GetRenderDataBuilder();
		pRenderDataBuilder->SetScene(pNewScene);
		pRenderDataBuilder->LoadInNeccessaryDataFromDisk();
		pRenderDataBuilder->ResolveRenderData();
		// push an event to notify the render device
		mMessageBusMember.PushEvent("RenderDataRebuild");

		Input::ForceReleaseAllEvent(); // to avoid short cut key status
		mSelectedEntity = {};
	}

}