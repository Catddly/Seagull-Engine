#include "StdAfx.h"
#include "Asset/Asset.h"

#include "System/FileSystem.h"

#include "Archive/ResourceLoader.h"

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

	TextureAsset::TextureAsset()
		:AssetBase("", EAssetType::eTexture), mFilename(""), mbNeedMipmap(false), mbIsCubeMap(false)
	{}

	TextureAsset::TextureAsset(const string& name, const string& filename, bool needMipmap, bool isCubeMap)
		: AssetBase(name, EAssetType::eTexture), mFilename(filename), mbNeedMipmap(needMipmap), mbIsCubeMap(isCubeMap)
	{}

	TextureAsset::TextureAsset(const string& name, const string& filename, Byte* pData, UInt32 byteSize, bool needMipmap, bool isCubeMap)
		: AssetBase(name, EAssetType::eTexture), mFilename(filename), mbNeedMipmap(needMipmap), mbIsCubeMap(isCubeMap)
	{
		// load data from memory immediately
		SG_ASSERT(!mTextureData.IsValid());
		TextureResourceLoader texLoader;
		if (!texLoader.LoadFromMemory(pData, byteSize, mTextureData, mbNeedMipmap, mbIsCubeMap))
		{
			SG_LOG_ERROR("Failed to load data from memory: %s, %s", GetAssetName().c_str(), mFilename.c_str());
			SG_ASSERT(false);
		}
	}

	void TextureAsset::LoadDataFromFile() noexcept
	{
		if (!mTextureData.IsValid())
		{
			TextureResourceLoader texLoader;
			if (!texLoader.LoadFromFile(mFilename.c_str(), mTextureData, mbNeedMipmap, mbIsCubeMap))
			{
				SG_LOG_ERROR("Failed to load data from file: %s", mFilename.c_str());
				SG_ASSERT(false);
			}
		}
	}

	string TextureAsset::GetFilePath() const noexcept
	{
		string path = FileSystem::GetResourceFolderPath(EResourceDirectory::eTextures, SG_ENGINE_DEBUG_BASE_OFFSET);
		path += mFilename;
		return eastl::move(path);
	}

	void TextureAsset::Serialize(json& node)
	{
		node["Name"] = GetAssetName().c_str();
		node["Filename"] = GetFileName().c_str();
		node["NeedMipmap"] = mbNeedMipmap;
		node["IsCubeMap"] = mbIsCubeMap;
	}

	void TextureAsset::Deserialize(json& node)
	{
		SetAssetName(node["Name"].get<std::string>().c_str());
		mFilename = node["Filename"].get<std::string>().c_str();
		node["NeedMipmap"].get_to(mbNeedMipmap);
		node["IsCubeMap"].get_to(mbIsCubeMap);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MaterialAsset
	////////////////////////////////////////////////////////////////////////////////////////////////////

	MaterialAsset::MaterialAsset()
		:AssetBase("", EAssetType::eMaterial), mFilename("")
	{}

	MaterialAsset::MaterialAsset(const string& name, const string& filename)
		: AssetBase(name, EAssetType::eMaterial), mFilename(filename)
	{}

}