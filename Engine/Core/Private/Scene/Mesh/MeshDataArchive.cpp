#include "StdAfx.h"
#include "Scene/Mesh/MeshDataArchive.h"

#include "System/Logger.h"

namespace SG
{

	UInt32 MeshDataArchive::msCurrKey = 0;

	UInt32 MeshDataArchive::AddRef(UInt32 meshId)
	{
		auto node = mMeshDatas.find(meshId);
		if (node != mMeshDatas.end())
		{
			return node->second.numReference++;
		}
		return UInt32(-1);
	}

	UInt32 MeshDataArchive::DecreaseRef(UInt32 meshId)
	{
		auto node = mMeshDatas.find(meshId);
		if (node != mMeshDatas.end())
		{
			UInt32 refCnt = --node->second.numReference;
			if (node->second.numReference == 0) // no mesh references this data
				mMeshDatas.erase(node);
			return refCnt;
		}
		return UInt32(-1);
	}

	UInt32 MeshDataArchive::SetData(const MeshData& meshData)
	{
		auto& md = const_cast<MeshData&>(meshData);
		md.numReference = 0;
		mMeshDatas[msCurrKey] = md;
		SG_LOG_DEBUG("New Mesh Data");
		return msCurrKey++;
	}

	const MeshData* MeshDataArchive::GetData(UInt32 meshId) const
	{
		auto node = mMeshDatas.find(meshId);
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