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

	TextureAsset::TextureAsset()
		:AssetBase("", EAssetType::eTexture), mFilename(""), mbNeedMipmap(false), mbIsCubeMap(false)
	{
	}

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

	void TextureAsset::Serialize()
	{
		//outStream << YAML::Key << "Name" << YAML::Value << GetAssetName().c_str();
		//outStream << YAML::Key << "Filename" << YAML::Value << GetFileName().c_str();
		//outStream << YAML::Key << "NeedMipmap" << YAML::Value << mbNeedMipmap;
		//outStream << YAML::Key << "IsCubeMap" << YAML::Value << mbIsCubeMap;
		//return true;
	}

	void TextureAsset::Deserialize()
	{
		//SetAssetName(node["Name"].as<std::string>().c_str());
		//mFilename = node["Filename"].as<std::string>().c_str();
		//mbNeedMipmap = node["NeedMipmap"].as<bool>();
		//mbIsCubeMap = node["IsCubeMap"].as<bool>();
	}

}