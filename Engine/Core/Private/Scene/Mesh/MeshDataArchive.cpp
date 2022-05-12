#include "StdAfx.h"
#include "Scene/Mesh/MeshDataArchive.h"

#include "System/Logger.h"

namespace SG
{

	IDAllocator<UInt32> MeshDataArchive::msIdAllocator;

	UInt32 MeshDataArchive::SetData(const MeshData& meshData)
	{
		UInt32 meshId = UInt32(-1);
		if (mMeshIDMap.find(meshData.filename) == mMeshIDMap.end())// this is a new mesh data
		{
			meshId = msIdAllocator.Allocate();
			//auto& md = const_cast<MeshData&>(meshData);
			mMeshDatas.emplace_back(meshData, 1);
			mMeshIDMap[meshData.filename] = meshId;
		}
		else
		{
			meshId = mMeshIDMap[meshData.filename];
			mMeshDatas[meshId].second += 1;
		}
		return meshId;
	}

	UInt32 MeshDataArchive::GetRefCount(UInt32 meshId) const
	{
		SG_ASSERT(meshId < mMeshDatas.size());
		return mMeshDatas[meshId].second;
	}

	const MeshData* MeshDataArchive::GetData(UInt32 meshId) const
	{
		SG_ASSERT(meshId < mMeshDatas.size());
		return &mMeshDatas[meshId].first;
	}

	UInt32 MeshDataArchive::GetMeshID(const string& filename) const
	{
		auto node = mMeshIDMap.find(filename);
		if (node != mMeshIDMap.end())
			return node->second;
		return UInt32(-1);
	}

	bool MeshDataArchive::HaveInstance(UInt32 meshId) const
	{
		UInt32 cnt = GetRefCount(meshId);
		return cnt > 1;
	}

	bool MeshDataArchive::HaveMeshData(const string& filename)
	{
		if (mMeshIDMap.find(filename) != mMeshIDMap.end())
			return true;
		return false;
	}

	void MeshDataArchive::Reset()
	{
		for (auto& meshData : mMeshDatas)
			meshData.second = 0;
	}

	void MeshDataArchive::LogDebugInfo()
	{
		SG_LOG_DEBUG("MeshDataArchive Debug Info:");
		UInt32 id = 0;
		for (auto& meshData : mMeshDatas)
		{
			SG_LOG_DEBUG("Mesh ID: %d", id);
			SG_LOG_DEBUG("    Filename: %s", meshData.first.filename.c_str());
			SG_LOG_DEBUG("    Ref Count: %d", meshData.second);
			++id;
		}
	}

	MeshDataArchive* MeshDataArchive::GetInstance()
	{
		static MeshDataArchive sInstance;
		return &sInstance;
	}

}