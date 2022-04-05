#pragma once

#include "System/System.h"
#include "System/LayerSystem/LayerSystem.h"

namespace SG
{

	class TestGUILayer : public ILayer
	{
	public:
		TestGUILayer()
			:ILayer("TestGUILayer")
		{}

		virtual void OnUpdate(float deltaTime);
	private:
		void DrawLightPanel();
		void DrawStatistics(float deltaTime);
	private:
		UInt32 mLastFps = 0;
		float mElapsedTime = 0.0;
	};

}