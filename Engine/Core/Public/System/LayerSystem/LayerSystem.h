#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"

#include "Stl/vector.h"
#include "Stl/string.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	class ILayer
	{
	public:
		ILayer(const char* name)
			:mName(name)
		{}
		virtual ~ILayer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate(float deltaTime) = 0;
	private:
		string mName;
	};

	class LayerSystem
	{
	public:
		SG_CORE_API void PushLayer(RefPtr<ILayer> pLayer);
		SG_CORE_API void PopLayer(RefPtr<ILayer> pLayer);

		SG_CORE_API void OnUpdate(float deltaTime);
	private:
		vector<RefPtr<ILayer>> mLayerStack;
	};

}