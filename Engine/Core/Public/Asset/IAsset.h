#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

#include "Archive/IDiskResource.h"

namespace SG
{

	enum class EAssetType
	{
		eUnknown = 0,
		eTexture,
	};

	interface IAsset : public IDiskResource
	{
		virtual ~IAsset() = default;

		virtual UInt32 GetAssetID() const = 0;
		virtual EAssetType GetAssetType() const = 0;
		virtual string GetAssetName() const = 0;
	};

}