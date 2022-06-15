#include "StdAfx.h"
#include "Archive/ResourceDefs.h"

#include "ktx/ktx.h"

namespace SG
{

	bool Raw2DTexture::IsValid() const noexcept
	{
		return pData != nullptr;
	}

	void Raw2DTexture::FreeMemory() noexcept
	{
		if (type == ETextureType::eKTX)
		{
			ktxTexture_Destroy(reinterpret_cast<ktxTexture*>(pUserData));
			pUserData = nullptr;
		}
		else
			free(pData);
		pData = nullptr;
	}

}