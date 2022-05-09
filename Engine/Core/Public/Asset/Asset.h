#pragma once

#include "Archive/ISerializable.h"
#include "Archive/IDAllocator.h"
#include "Scene/ResourceLoader/ResourceDefs.h"
#include "Asset/IAsset.h"

#include "Stl/string.h"

namespace SG
{

	class AssetBase : public IAsset
	{
	public:
		explicit AssetBase(const string& name, EAssetType type);
		virtual ~AssetBase();

		virtual string GetAssetFilePath() const noexcept = 0;

		UInt32 GetAssetID() const override { return mAssetId; }
		EAssetType GetAssetType() const override { return mAssetType; }

		string GetAssetName() const override { return mAssetName; }
	protected:
		void SetAssetName(const string& name) override { mAssetName = name; };
	private:
		static IDAllocator<UInt32> msAssetIdAllocator;
		string mAssetName = "";
		UInt32 mAssetId = IDAllocator<UInt32>::INVALID_ID;
		EAssetType mAssetType = EAssetType::eUnknown;
	};

	class TextureAsset final : public AssetBase, public ISerializable
	{
	public:
		TextureAsset();
		TextureAsset(const string& name, const string& filename, bool needMipmap = false, bool isCubeMap = false);

		virtual string GetFileName() const noexcept { return mFilename; }
		virtual string GetFilePath() const noexcept;

		SG_INLINE UInt32 GetWidth() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.width; }
		SG_INLINE UInt32 GetHeight() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.height; }

		SG_INLINE UInt32 GetArray() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.array; }
		SG_INLINE UInt32 GetMipmap() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.mipLevel; }
		SG_INLINE UInt32 GetDimention() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.dimention; }
		
		SG_INLINE UInt32 GetByteSize() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.sizeInByte; }
		SG_INLINE ETextureType GetType() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.type; }

		SG_INLINE unsigned char* GetRawData() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.pData; }
		SG_INLINE void* GetUserData() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.pUserData; }

		SG_INLINE bool IsValid() const noexcept { return mTextureData.IsValid(); }
		SG_INLINE void FreeMemory() noexcept { SG_ASSERT(mTextureData.IsValid()); mTextureData.FreeMemory(); }

		virtual void Serialize(json& node) override;
		virtual void Deserialize(json& node) override;
		
		virtual string GetAssetFilePath() const noexcept override { return mFilename; }
		virtual void   LoadDataFromDisk() noexcept override;
		virtual bool   IsDiskResourceLoaded() const noexcept { return mTextureData.IsValid(); };
	private:
		string       mFilename;
		Raw2DTexture mTextureData; //! For now, TextureAsset only refs to 2d textures.
		
		bool mbNeedMipmap = false;
		bool mbIsCubeMap = false;
	};

}