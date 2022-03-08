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
	};

}