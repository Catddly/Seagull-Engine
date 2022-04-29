#include "StdAfx.h"
#include "Asset/Asset.h"

#include "System/FileSystem.h"

#include "Scene/ResourceLoader/RenderResourceLoader.h"

namespace SG
{

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// AssetBase
	////////////////////////////////////////////////////////////////////////////////////////////////////

	IDAllocator<UInt32> AssetBase::msAssetIdAllocator;

	AssetBase::AssetBase(const string& name, EAssetType type)
		:mAssetName(name), mAssetType(type)
	{
		mAssetId = msAssetIdAllocator.Allocate();
	}

	AssetBase::~AssetBase()
	{
		msAssetIdAllocator.Restore(mAssetId);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// TextureAsset
	////////////////////////////////////////////////////////////////////////////////////////////////////

	TextureAsset::TextureAsset(const string& name, const string& filename, bool needMipmap, bool isCubeMap)
		:AssetBase(name, EAssetType::eTexture), mFilename(filename), mbNeedMipmap(needMipmap), mbIsCubeMap(isCubeMap)
	{}

	void TextureAsset::LoadDataFromDisk() noexcept
	{
		SG_ASSERT(!mTextureData.IsValid());
		TextureResourceLoader texLoader;
		if (!texLoader.LoadFromFile(mFilename.c_str(), mTextureData, mbNeedMipmap, mbIsCubeMap))
		{
			SG_LOG_ERROR("Failed to load data from file: %s", mFilename.c_str());
			SG_ASSERT(false);
		}
	}

	string TextureAsset::GetFilePath() const noexcept
	{
		string path = FileSystem::GetResourceFolderPath(EResourceDirectory::eTextures, SG_ENGINE_DEBUG_BASE_OFFSET);
		path += mFilename;
		return eastl::move(path);
	}

}