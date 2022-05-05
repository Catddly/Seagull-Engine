#include "StdAfx.h"
#include "Scene/Mesh/TextureAssetArchive.h"

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
		mTextureAssets[filename] = MakeRef<TextureAsset>(name, filename, needMipmap, isCubeMap);
		return mTextureAssets[filename];
	}

	TextureAssetArchive* TextureAssetArchive::GetInstance()
	{
		static TextureAssetArchive sInstance;
		return &sInstance;
	}

}