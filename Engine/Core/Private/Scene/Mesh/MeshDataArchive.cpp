#include "StdAfx.h"
#include "Scene/Mesh/MeshDataArchive.h"

#include "System/Logger.h"

namespace SG
{

	UInt32 MeshDataArchive::msCurrKey = 0;

	UInt32 MeshDataArchive::SetData(const MeshData& meshData)
	{
		mMeshDatas[msCurrKey] = meshData;
		SG_LOG_DEBUG("New Mesh Data");
		return msCurrKey++;
	}

	const MeshData* MeshDataArchive::GetData(UInt32 key) const
	{
		auto node = mMeshDatas.find(key);
		if (node == mMeshDatas.end())
			return nullptr;
		return &node->second;
	}

	MeshDataArchive* MeshDataArchive::GetInstance()
	{
		static MeshDataArchive sInstance;
		return &sInstance;
	}

}