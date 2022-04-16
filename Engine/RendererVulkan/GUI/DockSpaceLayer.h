#pragma once

#include "System/LayerSystem/LayerSystem.h"
#include "Event/MessageBus/MessageBus.h"

#include "Math/MathBasic.h"

namespace SG
{

	class DockSpaceLayer : public ILayer
	{
	public:
		DockSpaceLayer()
			:ILayer("Dockspace")
		{}

		virtual void OnAttach();

		virtual void OnUpdate(float deltaTime);
	private:
		void DrawDockSpaceBackground();
		void DrawMainViewport();
		void DrawLightPanel();
		void DrawStatistics(float deltaTime);
	private:
		MessageBusMember mMessageBusMember;

		UInt32 mLastFps = 0;
		float mElapsedTime = 0.0;

		Vector2f mLastViewportSize = { 0.0f, 0.0f };

		void* mpViewportTex = nullptr;
		void* mpLogoTex = nullptr;
	};

}