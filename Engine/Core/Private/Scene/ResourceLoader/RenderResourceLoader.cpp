#include "StdAfx.h"
#include "Scene/ResourceLoader/RenderResourceLoader.h"

#include "Render/SwapChain.h"
#include "System/FileSystem.h"

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
			outRaw.mipLevel = CalcMipmapLevel(outRaw.width, outRaw.height);
			outRaw.sizeInByte = width * height * numChannels;
			outRaw.dimention = numChannels;

			if (type == ETextureType::eUnknown)
				return false;

			outRaw.type = type;
			outRaw.pData = pData;
		}
		return true;
	}

	bool MeshResourceLoader::LoadFromFile(const char* name, vector<float>& vertices, vector<UInt32>& indices)
	{
		SG_PROFILE_FUNCTION();

		string path = FileSystem::GetResourceFolderPath(EResourceDirectory::eMeshes, SG_ENGINE_DEBUG_BASE_OFFSET);
		path += name;

		Assimp::Importer importer;
		auto* scene = importer.ReadFile(path.c_str(), aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			SG_LOG_ERROR(importer.GetErrorString());
			return false;
		}

		SG_LOG_DEBUG("Meshes: %d", scene->mNumMeshes);
		if (scene->HasMeshes())
		{
			//SG_LOG_DEBUG("Mesh Name: %s", scene->mRootNode->mName);
			for (UInt32 i = 0; i < scene->mNumMeshes; ++i)
			{
				const aiMesh* pMesh = scene->mMeshes[i];
				const UInt32  meshNumVertices = pMesh->mNumVertices;

				SG_ASSERT(pMesh->HasNormals());

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

					if (pMesh->HasTextureCoords(0))
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

				for (UInt32 index = 0; index < pMesh->mNumFaces; ++index)
				{
					const aiFace& pFace = pMesh->mFaces[index];
					for (UInt32 j = 0; j < pFace.mNumIndices; ++j)
					{
						indices.emplace_back(pFace.mIndices[j]);
					}
				}

				SG_LOG_DEBUG("Mesh Verticies: %d", vertices.size());
				SG_LOG_DEBUG("Mesh Indices  : %d", indices.size());
			}
		}
		else
			SG_LOG_WARN("Empty Mesh: %s", name);

		return true;
	}

}