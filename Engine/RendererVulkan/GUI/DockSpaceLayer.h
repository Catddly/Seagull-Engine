#pragma once

#include "System/LayerSystem/LayerSystem.h"

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
		UInt32 mLastFps = 0;
		float mElapsedTime = 0.0;

		void* mpViewportTex = nullptr;
		void* mpLogoTex = nullptr;
	};

}