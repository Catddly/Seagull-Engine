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

	void RenderDataBuilder::OnUpdate(float deltaTime)
	{
		SG_PROFILE_FUNCTION();

		if (!mCurrentFrameNewAssets.empty())
			mCurrentFrameNewAssets.clear();
	}

	void RenderDataBuilder::LogDebugInfo() const
	{
		SG_LOG_DEBUG("RenderDataBuilder Debug Info:");
		for (auto& buildData : mRenderMeshBuildDataMap)
		{
			SG_LOG_DEBUG("Mesh ID: %d", buildData.first);
			SG_LOG_DEBUG("    Object ID: %d", buildData.second.objectId);
			SG_LOG_DEBUG("    Instance Cnt: %d", buildData.second.instanceCount);
		}
	}

	void RenderDataBuilder::SetScene(WeakRefPtr<Scene> pScene)
	{
		SG_PROFILE_FUNCTION();

		mpScene = pScene;
		mbIsRenderDataReady = false;

		mRenderMeshBuildDataMap.clear();
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
					if (mat.albedoTex)
					{
						if (mAssets.find(mat.albedoTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[mat.albedoTex->GetAssetID()] = mat.albedoTex;
							mAssets[mat.albedoTex->GetAssetID()] = mat.albedoTex;
						}
					}
					if (mat.normalTex)
					{
						if (mAssets.find(mat.normalTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[mat.normalTex->GetAssetID()] = mat.normalTex;
							mAssets[mat.normalTex->GetAssetID()] = mat.normalTex;
						}
					}
					if (mat.metallicTex)
					{
						if (mAssets.find(mat.metallicTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[mat.metallicTex->GetAssetID()] = mat.metallicTex;
							mAssets[mat.metallicTex->GetAssetID()] = mat.metallicTex;
						}
					}
					if (mat.roughnessTex)
					{
						if (mAssets.find(mat.roughnessTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[mat.roughnessTex->GetAssetID()] = mat.roughnessTex;
							mAssets[mat.roughnessTex->GetAssetID()] = mat.roughnessTex;
						}
					}
					if (mat.AOTex)
					{
						if (mAssets.find(mat.AOTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[mat.AOTex->GetAssetID()] = mat.AOTex;
							mAssets[mat.AOTex->GetAssetID()] = mat.AOTex;
						}
					}
				}
			});

		for (auto& node : mCurrentFrameNewAssets)
		{
			auto pLock = node.second.lock();
			pLock->LoadDataFromDisk();
		}
	}

	void RenderDataBuilder::ResolveRenderData()
	{
		SG_PROFILE_FUNCTION();

		auto pScene = mpScene.lock();
		if (!pScene)
			SG_LOG_ERROR("No scene is set to build the render data!");

		// reset render status
		mRenderMeshBuildDataMap.clear();

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
				node.second.perInstanceData.set_capacity(0);
		}

		LogDebugInfo();

		mbIsRenderDataReady = true;
	}

}