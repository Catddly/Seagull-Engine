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

		SG_CORE_API static TextureAssetArchive* GetInstance();
	private:
		TextureAssetArchive() = default;
	private:
		eastl::unordered_map<string, RefPtr<TextureAsset>> mTextureAssets; // filepath -> texture asset
	};

}