#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "Stl/string.h"
#include "eastl/unordered_map.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	class TextureAsset;

	class TextureAssetArchive
	{
	public:
		RefPtr<TextureAsset> NewTextureAsset(const string& name, const string& filename, bool needMipmap = false, bool isCubeMap = false);
		RefPtr<TextureAsset> NewTextureAsset(const string& name, const string& filename, Byte* pData, UInt32 byteSize, bool needMipmap = false, bool isCubeMap = false);

		RefPtr<TextureAsset> GetTextureAsset(const string& assetName);
		RefPtr<TextureAsset> GetTextureAsset(UInt32 assetId);

		SG_CORE_API static TextureAssetArchive* GetInstance();
	private:
		TextureAssetArchive() = default;
	private:
		eastl::unordered_map<string, RefPtr<TextureAsset>> mTextureAssets; // assetName -> texture asset
		eastl::unordered_map<UInt32, string> mTextureAssetIdsMap;          // assetId -> assetName
	};

}