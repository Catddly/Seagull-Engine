#pragma once

#include "Defs/Defs.h"

#include "System/LayerSystem/LayerSystem.h"

namespace SG
{

	interface IGUIDriver
	{
	public:
		virtual ~IGUIDriver() = default;

		virtual bool OnInit() = 0;
		virtual void OnShutdown() = 0;

		virtual void OnUpdate(float deltaTime) = 0;

		void PushUserLayer(RefPtr<ILayer> pLayer) { mLayerSystem.PushLayer(pLayer); }
		void PopUserLayer(RefPtr<ILayer> pLayer) { mLayerSystem.PopLayer(pLayer); }
	protected:
		LayerSystem mLayerSystem;
	};

}