#include "StdAfx.h"
#include "System/LayerSystem/LayerSystem.h"

namespace SG
{

	void LayerSystem::PushLayer(RefPtr<ILayer> pLayer)
	{
		mLayerStack.push_back(pLayer);
		pLayer->OnAttach();
	}

	void LayerSystem::PopLayer(RefPtr<ILayer> pLayer)
	{
		for (auto beg = mLayerStack.begin(); beg != mLayerStack.end(); ++beg)
		{
			if (*beg == pLayer)
			{
				pLayer->OnDetach();
				mLayerStack.erase(beg);
				return;
			}
		}
	}

	void LayerSystem::OnUpdate(float deltaTime)
	{
		for (auto& pLayer : mLayerStack)
		{
			pLayer->OnUpdate(deltaTime);
		}
	}

}