#pragma once

#include "Base/Handle.h"
#include "System/Input.h"
#include "System/LayerSystem/LayerSystem.h"
#include "Scene/Scene.h"
#include "Event/MessageBus/MessageBus.h"

#include "Math/MathBasic.h"

namespace SG
{

	class VulkanDescriptorSet;

	class DockSpaceLayer : public ILayer, private IInputListener
	{
	public:
		DockSpaceLayer();
		~DockSpaceLayer();

		virtual void OnAttach();
		virtual void OnDetach();

		virtual void OnUpdate(float deltaTime);

		bool ShowStatisticsDetail() const { return mbShowStatisticsDetail; }
	private:
		virtual bool OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState) override;
		virtual bool OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos) override;
	private:
		void DrawDockSpaceBackground();
		void DrawMainViewport();
		void DrawSettingPanel();
		void DrawStatistics(float deltaTime);
		void DrawSceneHirerchy();
		void DrawSelectedEntityProperty();

		void DrawSaveSceneProgressBar();

		void OpenScene();
		void SaveScene();
		void SaveAsScene();
	private:
		MessageBusMember mMessageBusMember;

		UInt32 mLastFps = 0;
		UInt32 mFrameCounter = 1;
		float mElapsedTime = 0.0f;

		Vector2f mViewportPos = { 0.0f, 0.0f };
		Vector2f mLastViewportSize = { 0.0f, 0.0f };

		ReadOnlyHandle<VulkanDescriptorSet*> mViewportTexHandle;
		Scene::Entity mSelectedEntity;

		string mSavedSceneName = "default.scene";
		bool mbViewportOnFocused = false;
		bool mbViewportOnHovered = false;
		bool mbShowStatisticsDetail = true;
		bool mbShowSaveSceneProgressBar = false;
		bool mbTriggerSave = false;
		bool mbTriggerOpen = false;
		bool mbTriggerSaveAs = false;
	};

}