#include "StdAfx.h"
#include "Render/ResourceLoader/RenderResourceLoader.h"

#include "System/FileSystem.h"

// redefine memory allocation for stb_image.h
#define STBI_MALLOC(sz)        SG::Memory::Malloc(sz)
#define STBI_REALLOC(p, newsz) SG::Memory::Realloc(p, newsz)
#define STBI_FREE(p)           SG::Memory::Free(p)
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

#include "Stl/string.h"

namespace SG
{

	bool TextureResourceLoader::LoadFromFile(const char* name, Raw2DTexture& outRaw)
	{
		int width, height, numChannels;

		string path = FileSystem::GetResourceFolderPath(EResourceDirectory::eTextures);
		path += name;
		unsigned char* pData = stbi_load(path.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);

		outRaw.width = width;
		outRaw.height = height;
		outRaw.array = 1;
		outRaw.mipLevel = 1;
		
		auto type = GetResourceType(name);
		if (type == ETextureType::eUnknown)
			return false;

		outRaw.type  = type;
		outRaw.pData = pData;
		return true;
	}

}