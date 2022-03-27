#include "StdAfx.h"
#include "Scene/RenderDataBuilder.h"

#include "System/Logger.h"

namespace SG
{

	RenderDataBuilder::RenderDataBuilder(RefPtr<Scene> pScene)
		:mpScene(pScene)
	{
	}

	void RenderDataBuilder::SetScene(WeakRefPtr<Scene> pScene)
	{
		mpScene = pScene;
		mbIsRenderDataReady = false;
	}

	void RenderDataBuilder::BuildData()
	{
		auto pScene = mpScene.lock();
		if (!pScene)
			SG_LOG_ERROR("No scene is set to build the render data!");

		// collect instance info and set instance id.
		pScene->TraverseMesh([&](Mesh& mesh)
			{
				auto node = mRenderMeshBuildDataMap.find(mesh.GetMeshID());
				if (node == mRenderMeshBuildDataMap.end())
				{
					auto node = mRenderMeshBuildDataMap.insert(mesh.GetMeshID());
					node.first->second.objectId = mesh.GetObjectID();
					node.first->second.instanceCount = 1;
					mesh.mInstanceId = 0;
				}
				else // have instance
				{
					mesh.mInstanceId = node->second.instanceCount;
					node->second.instanceCount += 1;
				}
				auto& perInstanceData = mRenderMeshBuildDataMap[mesh.GetMeshID()].perInstanceData;
				perInstanceData.emplace_back(mesh.GetPosition(), mesh.GetScale().x);
			});

		for (auto node : mRenderMeshBuildDataMap) // clear the memory that is not necessary
		{
			if (node.second.instanceCount == 1)
				node.second.perInstanceData.set_capacity(0);
		}

		mbIsRenderDataReady = true;
	}

}