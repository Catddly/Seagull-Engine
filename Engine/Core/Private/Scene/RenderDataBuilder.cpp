#include "StdAfx.h"
#include "Scene/RenderDataBuilder.h"

#include "Scene/Mesh/MeshDataArchive.h"
#include "Profile/Profile.h"
#include "TipECS/Entity.h"

#include "System/Logger.h"

namespace SG
{

	RenderDataBuilder::RenderDataBuilder(RefPtr<Scene> pScene)
		:mpScene(pScene)
	{
	}

	void RenderDataBuilder::SetScene(WeakRefPtr<Scene> pScene)
	{
		SG_PROFILE_FUNCTION();

		mpScene = pScene;
		mbIsRenderDataReady = false;
	}

	void RenderDataBuilder::BuildData()
	{
		SG_PROFILE_FUNCTION();

		auto pScene = mpScene.lock();
		if (!pScene)
			SG_LOG_ERROR("No scene is set to build the render data!");

		// collect instance info and set instance id.
		pScene->TraverseEntity([this](auto& entity)
			{
				if (entity.HasComponent<MeshComponent>())
				{
					auto& meshComp = entity.GetComponent<MeshComponent>();
					auto meshId = meshComp.meshId;
					auto objectId = meshComp.objectId;

					auto node = mRenderMeshBuildDataMap.find(meshId);
					if (node == mRenderMeshBuildDataMap.end())
					{
						auto node = mRenderMeshBuildDataMap.insert(meshId);
						node.first->second.objectId = objectId;
						node.first->second.instanceCount = 1;
						meshComp.instanceId = 0;
					}
					else // have instance
					{
						meshComp.instanceId = node->second.instanceCount;
						node->second.instanceCount += 1;
					}
					auto& perInstanceData = mRenderMeshBuildDataMap[meshId].perInstanceData;
					perInstanceData.emplace_back(objectId);
				}
			});

		for (auto node : mRenderMeshBuildDataMap) // clear the memory that is not necessary
		{
			if (node.second.instanceCount == 1)
			{
				node.second.perInstanceData.set_capacity(0);
				MeshDataArchive::GetInstance()->SetFlag(node.first, false);
			}
		}

		mbIsRenderDataReady = true;
	}

}