#include "StdAfx.h"
#include "Archive/ResourceLoader.h"

#include "Render/SwapChain.h"
#include "System/FileSystem.h"
#include "Asset/Asset.h"
#include "Archive/TextureAssetArchive.h"
#include "Archive/MaterialAssetArchive.h"

// redefine memory allocation for stb_image.h
//#define STBI_MALLOC(sz)        Malloc(sz)
//#define STBI_REALLOC(p, newsz) Realloc(p, newsz)
//#define STBI_FREE(p)           Free(p)

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

//#undef STBI_MALLOC
//#undef STBI_REALLOC
//#undef STBI_FREE

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/vector3.h"
#include "assimp/mesh.h"

// TODO: memory allocation consistency
#include "ktx/ktx.h"
#include "ktx/ktxvulkan.h"

#include "Stl/string.h"
#include "Profile/Profile.h"

#include "Math/MathBasic.h"

#include "eastl/array.h"

namespace SG
{

	bool TextureResourceLoader::LoadFromFile(const char* name, Raw2DTexture& outRaw, bool bNeedMipMap, bool bIsCubeMap)
	{
		SG_PROFILE_FUNCTION();

		auto type = GetResourceType(name);
		string path = FileSystem::GetResourceFolderPath(EResourceDirectory::eTextures, SG_ENGINE_DEBUG_BASE_OFFSET);
		path += name;

		if (type == ETextureType::eKTX)
		{
			ktxTexture* pTexture;
			auto error = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pTexture);
			if (error != KTX_SUCCESS)
			{
				SG_LOG_ERROR("Failed to load in ktx texture: %s", name);
				return false;
			}

			outRaw.width = pTexture->baseWidth;
			outRaw.height = pTexture->baseHeight;
			outRaw.array = pTexture->numFaces;
			outRaw.mipLevel = pTexture->numLevels;
			outRaw.pData = pTexture->pData;
			outRaw.type = ETextureType::eKTX;
			outRaw.sizeInByte = static_cast<UInt32>(pTexture->dataSize);
			outRaw.pUserData = pTexture;

			if (pTexture->glInternalformat == 0x8229) // GL_R8
				outRaw.dimention = 1;
			else if (pTexture->glInternalformat == 0x822B) // GL_RG8
				outRaw.dimention = 2;
			else if (pTexture->glInternalformat == 0x8051) // GL_RGB8
				outRaw.dimention = 3;
			else if (pTexture->glInternalformat == 0x8058) // GL_RGBA8
				outRaw.dimention = 4;
		}
		else
		{
			int width, height, numChannels;
			unsigned char* pData = stbi_load(path.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);

			outRaw.width = bIsCubeMap ? (width / 4) : width;
			outRaw.height = bIsCubeMap ? (height / 3) : height;
			outRaw.array = bIsCubeMap ? 6 : 1;
			outRaw.mipLevel = bNeedMipMap ? CalcMipmapLevel(outRaw.width, outRaw.height) : 1;
			outRaw.sizeInByte = width * height * numChannels;
			outRaw.dimention = numChannels;

			if (type == ETextureType::eUnknown)
				return false;

			outRaw.type = type;
			outRaw.pData = pData;
		}
		return true;
	}

	bool TextureResourceLoader::LoadFromMemory(Byte* pData, UInt32 byteSize, Raw2DTexture& outRaw, bool bNeedMipMap, bool bIsCubeMap)
	{
		SG_PROFILE_FUNCTION();

		int x, y, comp;
		unsigned char* pOutData = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(pData), (int)byteSize, &x, &y, &comp, STBI_rgb_alpha);

		outRaw.width = bIsCubeMap ? (x / 4) : x;
		outRaw.height = bIsCubeMap ? (y / 3) : y;
		outRaw.array = bIsCubeMap ? 6 : 1;
		outRaw.mipLevel = bNeedMipMap ? CalcMipmapLevel(outRaw.width, outRaw.height) : 1;

		// force to be 4 channels
		outRaw.sizeInByte = x * y * STBI_rgb_alpha;
		outRaw.dimention = STBI_rgb_alpha;

		outRaw.type = ETextureType::eJPG;
		outRaw.pData = pOutData;

		return true;
	}

	bool MeshResourceLoader::LoadFromFile(const string& name, EMeshType type, MeshData& meshData, bool bLoadMaterials)
	{
		SG_PROFILE_FUNCTION();

		SG_ASSERT(meshData.subMeshDatas.empty() && "It must be an empty mesh data!");

		meshData.filename = name;

		string fullName = name;
		fullName += MeshTypeToExtString(type);

		string path = FileSystem::GetResourceFolderPath(EResourceDirectory::eMeshes, SG_ENGINE_DEBUG_BASE_OFFSET);
		path += fullName;

		Assimp::Importer importer;
		auto* scene = importer.ReadFile(path.c_str(), aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			SG_LOG_ERROR(importer.GetErrorString());
			return false;
		}

#define MACRO_EXPAND_LOAD_MATERIAL_TEXTURE(MAT_NAME, STORED_ARRAY, COUNTER, TEXTURE_TYPE) \
		SG_LOG_DEBUG("%s Texture Size: %d", #TEXTURE_TYPE, MAT_NAME->GetTextureCount(TEXTURE_TYPE)); \
		for (UInt32 i = 0; i < MAT_NAME->GetTextureCount(TEXTURE_TYPE); ++i) \
		{ \
			aiString texName; \
			MAT_NAME->GetTexture(TEXTURE_TYPE, i, &texName); \
			auto* pTex = scene->GetEmbeddedTexture(texName.C_Str()); \
			string textureName = "embedded_texture_"; \
			textureName += texName.C_Str(); \
			STORED_ARRAY[COUNTER] = TextureAssetArchive::GetInstance()->NewTextureAsset(textureName, name, reinterpret_cast<Byte*>(pTex->pcData), pTex->mWidth, true); \
			SG_LOG_DEBUG("    Name: %s", texName.C_Str()); \
			SG_LOG_DEBUG("    Width: %d", pTex->mWidth); \
			SG_LOG_DEBUG("    Height: %d", pTex->mHeight); \
			SG_LOG_DEBUG("    Format: %s", pTex->achFormatHint); \
		} \
		++COUNTER;

#define LOAD_TEXTURE_TYPES(F, MAT_NAME, STORED_ARRAY, COUNTER) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_DIFFUSE) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_SPECULAR) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_AMBIENT) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_HEIGHT) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_NORMALS) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_LIGHTMAP) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_NORMAL_CAMERA) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_METALNESS) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_DIFFUSE_ROUGHNESS) \
		F(MAT_NAME, STORED_ARRAY, COUNTER, aiTextureType_AMBIENT_OCCLUSION) \

		vector<RefPtr<MaterialAsset>> loadInMaterials;

		if (bLoadMaterials)
		{
			SG_LOG_DEBUG("Loading Mesh Embedded Materials: Count of %d", scene->mNumMaterials);
			if (scene->HasMaterials())
			{
				for (UInt32 i = 0; i < scene->mNumMaterials; ++i)
				{
					auto* pMaterial = scene->mMaterials[i];
					if (strcmp(pMaterial->GetName().C_Str(), "") != 0)
					{
						SG_LOG_DEBUG("Loading Embedded Material: %s", pMaterial->GetName().C_Str());

						eastl::array<RefPtr<TextureAsset>, 10> loadInTextures = { nullptr };
						UInt32 textureCounter = 0;

						// load in all the textures in one material
						LOAD_TEXTURE_TYPES(MACRO_EXPAND_LOAD_MATERIAL_TEXTURE, pMaterial, loadInTextures, textureCounter);

						// load as material
						auto pMaterialAsset = MaterialAssetArchive::GetInstance()->NewMaterialAsset(pMaterial->GetName().C_Str(), name);

						if (loadInTextures[0]) // diffuse texture exists
							pMaterialAsset->SetAlbedoTexture(loadInTextures[0]);
						if (loadInTextures[7]) // metallic texture exists
							pMaterialAsset->SetMetallicTexture(loadInTextures[7]);
						if (loadInTextures[8]) // roughness texture exists
							pMaterialAsset->SetRoughnessTexture(loadInTextures[8]);
						if (loadInTextures[4]) // normal texture exists
							pMaterialAsset->SetNormalTexture(loadInTextures[4]);
						if (loadInTextures[9]) // ao texture exists
							pMaterialAsset->SetAOTexture(loadInTextures[9]);

						loadInMaterials.emplace_back(pMaterialAsset);

						//SG_LOG_DEBUG("Properties: %d", pMaterial->mNumProperties);
						//for (UInt32 i = 0; i < pMaterial->mNumProperties; ++i)
						//{
						//	auto* pProperty = pMaterial->mProperties[i];
						//	SG_LOG_DEBUG("Name: %s", pProperty->mKey.C_Str());
						//	if (pProperty->mType == aiPTI_Float)
						//	{
						//		for (UInt32 i = 0; i < pProperty->mDataLength / sizeof(float); ++i)
						//			SG_LOG_DEBUG("    Float: %.3f", *(reinterpret_cast<float*>(pProperty->mData) + i));
						//	}
						//	else if (pProperty->mType == aiPTI_Double)
						//	{
						//		for (UInt32 i = 0; i < pProperty->mDataLength / sizeof(double); ++i)
						//			SG_LOG_DEBUG("    Double: %.3f", *(reinterpret_cast<double*>(pProperty->mData) + i));
						//	}
						//	else if (pProperty->mType == aiPTI_String)
						//	{
						//		char str[256] = {};
						//		memcpy(str, pProperty->mData, pProperty->mDataLength * sizeof(char));
						//		str[pProperty->mDataLength + 1] = '\0';
						//		SG_LOG_DEBUG("    String: %s", str);
						//	}
						//	else if (pProperty->mType == aiPTI_Integer)
						//	{
						//		for (UInt32 i = 0; i < pProperty->mDataLength / sizeof(int); ++i)
						//			SG_LOG_DEBUG("    Int: %d", *(reinterpret_cast<int*>(pProperty->mData) + i));
						//	}
						//	else if (pProperty->mType == aiPTI_Buffer)
						//		SG_LOG_DEBUG("    Buffer: %d", pProperty->mDataLength);
						//}
					}
				}
			}
		}
#undef MACRO_EXPAND_LOAD_MATERIAL_TEXTURE
#undef LOAD_TEXTURE_TYPES

		SG_LOG_DEBUG("Loading Meshes: Count of %d", scene->mNumMeshes);
		if (scene->HasMeshes())
		{
			SG_ASSERT(loadInMaterials.empty() || loadInMaterials.size() == scene->mNumMeshes);

			for (UInt32 i = 0; i < scene->mNumMeshes; ++i)
			{
				auto& subMesh = meshData.subMeshDatas.emplace_back();
				subMesh.filename = name;
				subMesh.bIsProceduralMesh = false;
				if (bLoadMaterials)
					subMesh.materialAssetId = loadInMaterials[i]->GetAssetID();

				const aiMesh* pMesh = scene->mMeshes[i];
				subMesh.subMeshName = pMesh->mName.C_Str();
				SG_LOG_DEBUG("    Loading submesh: %s", subMesh.subMeshName.c_str());

				SG_ASSERT(pMesh->HasNormals());

				const UInt32 meshNumVertices = pMesh->mNumVertices;
				auto& vertices = subMesh.vertices;
				for (UInt32 index = 0; index < meshNumVertices; ++index)
				{
					const aiVector3D& vertexPos = pMesh->mVertices[index];
					vertices.emplace_back(vertexPos.x);
					vertices.emplace_back(vertexPos.y);
					vertices.emplace_back(vertexPos.z);

					const aiVector3D& vertexNormal = pMesh->mNormals[index];
					vertices.emplace_back(vertexNormal.x);
					vertices.emplace_back(vertexNormal.y);
					vertices.emplace_back(vertexNormal.z);

					if (pMesh->HasTextureCoords(0)) // default: uv channel 0 is for texture mapping
					{
						const aiVector3D& vertexUV = pMesh->mTextureCoords[0][index];
						vertices.emplace_back(vertexUV.x);
						vertices.emplace_back(vertexUV.y);
					}
					else
					{
						vertices.emplace_back(0.0f);
						vertices.emplace_back(0.0f);
					}

					if (pMesh->HasTangentsAndBitangents())
					{
						const aiVector3D& vertexTangent = pMesh->mTangents[index];
						vertices.emplace_back(vertexTangent.x);
						vertices.emplace_back(vertexTangent.y);
						vertices.emplace_back(vertexTangent.z);
					}
					else
					{
						vertices.emplace_back(0.0f);
						vertices.emplace_back(0.0f);
						vertices.emplace_back(0.0f);
					}
				}

				auto& indices = subMesh.indices;
				for (UInt32 index = 0; index < pMesh->mNumFaces; ++index)
				{
					const aiFace& pFace = pMesh->mFaces[index];
					for (UInt32 j = 0; j < pFace.mNumIndices; ++j)
					{
						indices.emplace_back(pFace.mIndices[j]);
					}
				}

				SG_LOG_DEBUG("    Mesh Verticies: %d", vertices.size());
				SG_LOG_DEBUG("    Mesh Indices  : %d", indices.size());
			}
		}
		else
			SG_LOG_WARN("Empty Mesh: %s", name);

		return true;
	}

}