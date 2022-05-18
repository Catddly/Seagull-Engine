#pragma once

#include "Archive/ISerializable.h"
#include "Archive/IDAllocator.h"
#include "Archive/ResourceDefs.h"
#include "Asset/IAsset.h"

#include "Math/MathBasic.h"

#include "Stl/SmartPtr.h"
#include "Stl/string.h"

namespace SG
{

	// this is the most traditional way to do this.
	// consider adding a property system.

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
		TextureAsset(const string& name, const string& filename, Byte* pData, UInt32 byteSize, bool needMipmap = false, bool isCubeMap = false);

		virtual string GetFileName() const noexcept { return mFilename; }
		virtual string GetFilePath() const noexcept;

		SG_INLINE UInt32 GetWidth() const noexcept { return mTextureData.width; }
		SG_INLINE UInt32 GetHeight() const noexcept { return mTextureData.height; }

		SG_INLINE UInt32 GetArray() const noexcept { return mTextureData.array; }
		SG_INLINE UInt32 GetMipmap() const noexcept { return mTextureData.mipLevel; }
		SG_INLINE UInt32 GetDimention() const noexcept {return mTextureData.dimention; }
		
		SG_INLINE UInt32 GetByteSize() const noexcept { return mTextureData.sizeInByte; }
		SG_INLINE ETextureType GetType() const noexcept { return mTextureData.type; }

		SG_INLINE unsigned char* GetRawData() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.pData; }
		SG_INLINE void* GetUserData() const noexcept { SG_ASSERT(mTextureData.IsValid()); return mTextureData.pUserData; }

		SG_INLINE bool IsValid() const noexcept { return mTextureData.IsValid(); }
		SG_INLINE void FreeMemory() noexcept { SG_ASSERT(mTextureData.IsValid()); mTextureData.FreeMemory(); }

		virtual void Serialize(json& node) override;
		virtual void Deserialize(json& node) override;
		
		virtual string GetAssetFilePath() const noexcept override { return mFilename; }
		virtual void   LoadDataFromFile() noexcept override;
		virtual bool   IsDiskResourceLoaded() const noexcept { return mTextureData.IsValid(); };
	private:
		string       mFilename;
		Raw2DTexture mTextureData; //! For now, TextureAsset only refs to 2d textures.
		
		bool mbNeedMipmap = false;
		bool mbIsCubeMap = false;
	};

	class MaterialAsset final : public AssetBase, public ISerializable
	{
	public:
		MaterialAsset();
		MaterialAsset(const string& name, const string& filename);

		virtual string GetFileName() const noexcept { return mFilename; }
		virtual string GetFilePath() const noexcept { return ""; }

		SG_INLINE void SetAlbedo(const Vector3f& value) noexcept { mAlbedo = value; }
		SG_INLINE void SetMetallic(float value) noexcept { mMetallic = value; }
		SG_INLINE void SetRoughness(float value) noexcept { mRoughness = value; }

		SG_INLINE Vector3f GetAlbedo() const noexcept { return mAlbedo; }
		SG_INLINE float    GetMetallic() const noexcept { return mMetallic; }
		SG_INLINE float    GetRoughness() const noexcept { return mRoughness; }

		SG_INLINE void SetAlbedoTexture(RefPtr<TextureAsset> pTex) noexcept { mAlbedoTex = pTex; }
		SG_INLINE void SetMetallicTexture(RefPtr<TextureAsset> pTex) noexcept { mMetallicTex = pTex; }
		SG_INLINE void SetRoughnessTexture(RefPtr<TextureAsset> pTex) noexcept { mRoughnessTex = pTex; }
		SG_INLINE void SetNormalTexture(RefPtr<TextureAsset> pTex) noexcept { mNormalTex = pTex; }
		SG_INLINE void SetAOTexture(RefPtr<TextureAsset> pTex) noexcept { mAOTex = pTex; }

		SG_INLINE RefPtr<TextureAsset> GetAlbedoTexture() const noexcept { return mAlbedoTex; }
		SG_INLINE RefPtr<TextureAsset> GetMetallicTexture() const noexcept { return mMetallicTex; }
		SG_INLINE RefPtr<TextureAsset> GetRoughnessTexture() const noexcept { return mRoughnessTex; }
		SG_INLINE RefPtr<TextureAsset> GetNormalTexture() const noexcept { return mNormalTex; }
		SG_INLINE RefPtr<TextureAsset> GetAOTexture() const noexcept { return mAOTex; }

		virtual void Serialize(json& node) override {}
		virtual void Deserialize(json& node) override {}

		virtual string GetAssetFilePath() const noexcept override { return mFilename; }
		virtual void   LoadDataFromFile() noexcept override {}
		virtual bool   IsDiskResourceLoaded() const noexcept { return true; };
	private:
		string mFilename;

		Vector3f mAlbedo = { 1.0f, 1.0f, 1.0f };
		float mMetallic = 0.1f;
		float mRoughness = 0.75f;

		RefPtr<TextureAsset> mAlbedoTex = nullptr;
		RefPtr<TextureAsset> mMetallicTex = nullptr;
		RefPtr<TextureAsset> mRoughnessTex = nullptr;
		RefPtr<TextureAsset> mNormalTex = nullptr;
		RefPtr<TextureAsset> mAOTex = nullptr;
	};

}