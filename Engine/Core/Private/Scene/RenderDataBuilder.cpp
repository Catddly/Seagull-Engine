#include "StdAfx.h"
#include "Scene/RenderDataBuilder.h"

#include "Scene/Components.h"
#include "Scene/Mesh/MeshDataArchive.h"
#include "Profile/Profile.h"
#include "TipECS/Entity.h"

#include "System/Logger.h"

namespace SG
{

	RenderDataBuilder::RenderDataBuilder(WeakRefPtr<Scene> pScene)
		:mpScene(pScene)
	{
	}

	void RenderDataBuilder::SetScene(WeakRefPtr<Scene> pScene)
	{
		SG_PROFILE_FUNCTION();

		mpScene = pScene;
		mbIsRenderDataReady = false;
	}

	void RenderDataBuilder::LoadInNeccessaryDataFromDisk()
	{
		SG_PROFILE_FUNCTION();

		auto pScene = mpScene.lock();
		if (!pScene)
			SG_LOG_ERROR("No scene is set to build the render data!");

		pScene->TraverseEntity([this](auto& entity)
			{
				// for now, only material component have asset that need to be load in.
				if (entity.HasComponent<MaterialComponent>())
				{
					MaterialComponent& mat = entity.GetComponent<MaterialComponent>();

					// TODO: may be there is a more automatic and smarter way to load asset.
					if (mat.albedoMap) mAssets.push_back(mat.albedoMap.get());
					if (mat.normalMap) mAssets.push_back(mat.normalMap.get());
					if (mat.metallicMap) mAssets.push_back(mat.metallicMap.get());
					if (mat.roughnessMap) mAssets.push_back(mat.roughnessMap.get());
					if (mat.AOMap) mAssets.push_back(mat.AOMap.get());
				}
			});

		for (auto* pAsset : mAssets)
			pAsset->LoadDataFromDisk();
	}

	void RenderDataBuilder::ResolveRenderData()
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