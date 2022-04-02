#include "StdAfx.h"
#include "Scene/Mesh/MeshDataArchive.h"

#include "System/Logger.h"

namespace SG
{

	UInt32 MeshDataArchive::msCurrKey = 0;

	UInt32 MeshDataArchive::SetData(const MeshData& meshData)
	{
		auto& md = const_cast<MeshData&>(meshData);
		mMeshDatas[msCurrKey] = { md, true };
		//SG_LOG_DEBUG("New Mesh Data");
		return msCurrKey++;
	}

	void MeshDataArchive::SetFlag(UInt32 meshId, bool bHaveInstance)
	{
		mMeshDatas[meshId].second = bHaveInstance;
	}

	const MeshData* MeshDataArchive::GetData(UInt32 meshId) const
	{
		auto node = mMeshDatas.find(meshId);
		if (node == mMeshDatas.end())
			return nullptr;
		return &node->second.first;
	}

	bool MeshDataArchive::HaveInstance(UInt32 meshId) const
	{
		auto node = mMeshDatas.find(meshId);
		if (node == mMeshDatas.end())
		{
			SG_LOG_ERROR("No mesh have id: %d", meshId);
			return false;
		}
		return node->second.second;
	}

	MeshDataArchive* MeshDataArchive::GetInstance()
	{
		static MeshDataArchive sInstance;
		return &sInstance;
	}

}