#pragma once

#include "Defs/Defs.h"

namespace SG
{

	class Scene;

	interface IGUIDriver
	{
	public:
		virtual ~IGUIDriver() = default;

		virtual bool OnInit() = 0;
		virtual void OnShutdown() = 0;

		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnDraw(Scene* pScene) = 0;
	};

}