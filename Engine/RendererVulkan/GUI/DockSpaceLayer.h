#pragma once

#include "Base/Handle.h"
#include "System/Input.h"
#include "System/LayerSystem/LayerSystem.h"
#include "Scene/Scene.h"
//#include "Event/MessageBus/MessageBus.h"

#include "RendererVulkan/Backend/VulkanDescriptor.h"

#include "Math/MathBasic.h"

namespace SG
{

	class DockSpaceLayer : public ILayer, private IInputListener
	{
	public:
		DockSpaceLayer();
		~DockSpaceLayer();

		virtual void OnAttach();

		virtual void OnUpdate(float deltaTime);
	private:
		virtual bool OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos) override;
	private:
		void DrawDockSpaceBackground();
		void DrawMainViewport();
		void DrawSettingPanel();
		void DrawStatistics(float deltaTime);
		void DrawSceneHirerchy();
		void DrawSelectedEntityProperty();
	private:
		//MessageBusMember mMessageBusMember;

		UInt32 mLastFps = 0;
		float mElapsedTime = 0.0;

		Vector2f mLastViewportSize = { 0.0f, 0.0f };

		Handle<VulkanDescriptorSet>* mpViewportTexHandle;
		Scene::Entity mSelectedEntity;

		bool mbViewportCanUpdateMouse = false;
	};

}