#include "StdAfx.h"
#include "Scene/RenderDataBuilder.h"

#include "Scene/Components.h"
#include "Archive/MeshDataArchive.h"
#include "Archive/MaterialAssetArchive.h"
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
					auto matAsset = mat.materialAsset.lock();
					const UInt32 matTextureMask = matAsset->GetTextureMask();

					// TODO: may be there is a more automatic and smarter way to load asset.
					if ((matTextureMask & MaterialAsset::ALBEDO_TEX_MASK) != 0)
					{
						auto albedoTex = matAsset->GetAlbedoTexture();
						if (mAssets.find(albedoTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[albedoTex->GetAssetID()] = albedoTex;
							mAssets[albedoTex->GetAssetID()] = albedoTex;
						}
					}
					if ((matTextureMask & MaterialAsset::NORMAL_TEX_MASK) != 0)
					{
						auto normalTex = matAsset->GetNormalTexture();
						if (mAssets.find(normalTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[normalTex->GetAssetID()] = normalTex;
							mAssets[normalTex->GetAssetID()] = normalTex;
						}
					}
					if ((matTextureMask & MaterialAsset::METALLIC_TEX_MASK) != 0)
					{
						auto metallicTex = matAsset->GetMetallicTexture();
						if (mAssets.find(metallicTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[metallicTex->GetAssetID()] = metallicTex;
							mAssets[metallicTex->GetAssetID()] = metallicTex;
						}
					}
					if ((matTextureMask & MaterialAsset::ROUGHNESS_TEX_MASK) != 0)
					{
						auto roughnessTex = matAsset->GetRoughnessTexture();
						if (mAssets.find(roughnessTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[roughnessTex->GetAssetID()] = roughnessTex;
							mAssets[roughnessTex->GetAssetID()] = roughnessTex;
						}
					}
					if ((matTextureMask & MaterialAsset::AO_TEX_MASK) != 0)
					{
						auto AOTex = matAsset->GetAOTexture();
						if (mAssets.find(AOTex->GetAssetID()) == mAssets.end())
						{
							mCurrentFrameNewAssets[AOTex->GetAssetID()] = AOTex;
							mAssets[AOTex->GetAssetID()] = AOTex;
						}
					}
				}
			});

		for (auto& node : mCurrentFrameNewAssets)
		{
			auto pLock = node.second.lock();
			pLock->LoadDataFromFile();
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
						auto& rendererBuildData = node.first->second;

						rendererBuildData.objectId = objectId;
						rendererBuildData.instanceCount = 1;
						meshComp.instanceId = 0;

						if (entity.HasComponent<MaterialComponent>())
						{
							MaterialComponent& matComp = entity.GetComponent<MaterialComponent>();
							auto materialAsset = matComp.materialAsset.lock();

							rendererBuildData.materialAssetName = materialAsset->GetAssetName();
							rendererBuildData.materialTextureMask = materialAsset->GetTextureMask();
							if ((rendererBuildData.materialTextureMask & MaterialAsset::ALBEDO_TEX_MASK) != 0)
								rendererBuildData.albedoTexAssetName = materialAsset->GetAlbedoTexture()->GetAssetName();
							if ((rendererBuildData.materialTextureMask & MaterialAsset::METALLIC_TEX_MASK) != 0)
								rendererBuildData.metallicTexAssetName = materialAsset->GetMetallicTexture()->GetAssetName();
							if ((rendererBuildData.materialTextureMask & MaterialAsset::ROUGHNESS_TEX_MASK) != 0)
								rendererBuildData.roughnessTexAssetName = materialAsset->GetRoughnessTexture()->GetAssetName();
							if ((rendererBuildData.materialTextureMask & MaterialAsset::NORMAL_TEX_MASK) != 0)
								rendererBuildData.normalTexAssetName = materialAsset->GetNormalTexture()->GetAssetName();
							if ((rendererBuildData.materialTextureMask & MaterialAsset::AO_TEX_MASK) != 0)
								rendererBuildData.aoTexAssetName = materialAsset->GetAOTexture()->GetAssetName();
						}
					}
					else // have instance
					{
						auto& rendererBuildData = node->second;
						meshComp.instanceId = rendererBuildData.instanceCount;
						rendererBuildData.instanceCount += 1;

						if (entity.HasComponent<MaterialComponent>())
						{
							MaterialComponent& matComp = entity.GetComponent<MaterialComponent>();
							auto materialAsset = matComp.materialAsset.lock();
							const UInt32 oldMask = rendererBuildData.materialTextureMask;

							const UInt32 textureMask = materialAsset->GetTextureMask();

							if ((textureMask | oldMask) != oldMask) // it is the new maximum set
							{
								rendererBuildData.materialAssetName = materialAsset->GetAssetName();
								rendererBuildData.materialTextureMask = textureMask;

								if ((textureMask & MaterialAsset::ALBEDO_TEX_MASK) != 0)
									rendererBuildData.albedoTexAssetName = materialAsset->GetAlbedoTexture()->GetAssetName();
								if ((textureMask & MaterialAsset::METALLIC_TEX_MASK) != 0)
									rendererBuildData.metallicTexAssetName = materialAsset->GetMetallicTexture()->GetAssetName();
								if ((textureMask & MaterialAsset::ROUGHNESS_TEX_MASK) != 0)
									rendererBuildData.roughnessTexAssetName = materialAsset->GetRoughnessTexture()->GetAssetName();
								if ((textureMask & MaterialAsset::NORMAL_TEX_MASK) != 0)
									rendererBuildData.normalTexAssetName = materialAsset->GetNormalTexture()->GetAssetName();
								if ((textureMask & MaterialAsset::AO_TEX_MASK) != 0)
									rendererBuildData.aoTexAssetName = materialAsset->GetAOTexture()->GetAssetName();
							}
						}
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