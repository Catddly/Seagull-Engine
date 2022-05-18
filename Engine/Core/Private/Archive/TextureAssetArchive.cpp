#include "StdAfx.h"
#include "Archive/TextureAssetArchive.h"

#include "Asset/Asset.h"

namespace SG
{

	RefPtr<TextureAsset> TextureAssetArchive::NewTextureAsset(const string& name, const string& filename, bool needMipmap, bool isCubeMap)
	{
		auto node = mTextureAssets.find(filename);
		if (node != mTextureAssets.end())
		{
			return node->second;
		}
		auto pNewTexture = MakeRef<TextureAsset>(name, filename, needMipmap, isCubeMap);
		mTextureAssets[filename] = pNewTexture;
		mTextureAssetIdsMap[pNewTexture->GetAssetID()] = filename;
		return pNewTexture;
	}

	RefPtr<TextureAsset> TextureAssetArchive::NewTextureAsset(const string& name, const string& filename, Byte* pData, UInt32 byteSize, bool needMipmap, bool isCubeMap)
	{
		// bacause here the filename is repeated, so we use name.
		auto node = mTextureAssets.find(name);
		if (node != mTextureAssets.end())
		{
			return node->second;
		}
		auto pNewTexture = MakeRef<TextureAsset>(name, filename, pData, byteSize, needMipmap, isCubeMap);
		mTextureAssets[name] = pNewTexture;
		mTextureAssetIdsMap[pNewTexture->GetAssetID()] = name;
		return pNewTexture;
	}

	RefPtr<TextureAsset> TextureAssetArchive::GetTextureAsset(const string& assetName)
	{
		auto node = mTextureAssets.find(assetName);
		if (node != mTextureAssets.end())
			return node->second;
		return nullptr;
	}

	RefPtr<TextureAsset> TextureAssetArchive::GetTextureAsset(UInt32 assetId)
	{
		auto node = mTextureAssetIdsMap.find(assetId);
		if (node == mTextureAssetIdsMap.end())
			return nullptr;
		return mTextureAssets[mTextureAssetIdsMap[assetId]];
	}

	TextureAssetArchive* TextureAssetArchive::GetInstance()
	{
		static TextureAssetArchive sInstance;
		return &sInstance;
	}

}