#include "StdAfx.h"
#include "Archive/MaterialAssetArchive.h"

#include "Asset/Asset.h"

namespace SG
{

	RefPtr<MaterialAsset> MaterialAssetArchive::NewMaterialAsset(const string& assetName, const string& filename)
	{
		auto node = mMaterialAssets.find(assetName);
		if (node != mMaterialAssets.end())
			return node->second;
		auto pNewMaterial = MakeRef<MaterialAsset>(assetName, filename);
		mMaterialAssets[assetName] = pNewMaterial;
		mMaterialAssetIdMap[pNewMaterial->GetAssetID()] = assetName;
		return pNewMaterial;
	}

	RefPtr<MaterialAsset> MaterialAssetArchive::GetMaterialAsset(const string& assetName)
	{
		auto node = mMaterialAssets.find(assetName);
		if (node == mMaterialAssets.end())
			return nullptr;
		return mMaterialAssets[assetName];
	}

	RefPtr<MaterialAsset> MaterialAssetArchive::GetMaterialAsset(UInt32 assetId)
	{
		auto node = mMaterialAssetIdMap.find(assetId);
		if (node == mMaterialAssetIdMap.end())
			return nullptr;
		return mMaterialAssets[mMaterialAssetIdMap[assetId]];
	}

	MaterialAssetArchive* MaterialAssetArchive::GetInstance()
	{
		static MaterialAssetArchive sInstance;
		return &sInstance;
	}

}