#pragma once

#include "Stl/string.h"
#include "Stl/SmartPtr.h"
#include "eastl/unordered_map.h"

namespace SG
{

	class MaterialAsset;

	class MaterialAssetArchive
	{
	public:
		RefPtr<MaterialAsset> NewMaterialAsset(const string& assetName, const string& filename);

		RefPtr<MaterialAsset> GetMaterialAsset(const string& assetName);
		RefPtr<MaterialAsset> GetMaterialAsset(UInt32 assetId);

		SG_CORE_API static MaterialAssetArchive* GetInstance();
	private:
		MaterialAssetArchive() = default;
	private:
		eastl::unordered_map<string, RefPtr<MaterialAsset>> mMaterialAssets; // assetName -> material asset
		eastl::unordered_map<UInt32, string> mMaterialAssetIdMap;            // assetId -> assetName
	};

}