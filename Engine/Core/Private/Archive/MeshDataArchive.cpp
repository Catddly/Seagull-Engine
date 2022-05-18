#include "StdAfx.h"
#include "Archive/MeshDataArchive.h"

#include "System/Logger.h"

#include "eastl/algorithm.h"

namespace SG
{

	IDAllocator<UInt32> MeshDataArchive::msIdAllocator;

	UInt32 MeshDataArchive::SetData(const SubMeshData& subMeshData)
	{
		UInt32 meshId = UInt32(-1);
		if (mMeshIDMap.find(subMeshData.subMeshName) == mMeshIDMap.end()) // this is a new mesh data
		{
			meshId = msIdAllocator.Allocate();
			mSubMeshDatas.emplace_back(subMeshData, 1);
			mMeshIDMap[subMeshData.subMeshName] = meshId;

			if (meshId == 0)
				mInstanceCountAreaSumTable.emplace_back(0);
			else
			{
				mInstanceCountAreaSumTable.emplace_back();
				mInstanceCountAreaSumTable[meshId] = mInstanceCountAreaSumTable[meshId - 1];
			}
		}
		else
		{
			meshId = mMeshIDMap[subMeshData.subMeshName];
			mSubMeshDatas[meshId].second += 1;
			// update the area sum table. (each element behind the meshId plus one)
			if (meshId > 0)
				mInstanceCountAreaSumTable[meshId] = mInstanceCountAreaSumTable[meshId - 1] + (GetRefCount(meshId) <= 1 ? 0 : GetRefCount(meshId));
			for (UInt32 i = meshId + 1; i < mInstanceCountAreaSumTable.size(); ++i)
				mInstanceCountAreaSumTable[i] = mInstanceCountAreaSumTable[i - 1] + (GetRefCount(i) <= 1 ? 0 : GetRefCount(i));
		}
		return meshId;
	}

	//UInt32 MeshDataArchive::SetData(const MeshData& meshData)
	//{
	//	UInt32 meshId = UInt32(-1);
	//	if (mMeshIDMap.find(meshData.filename) == mMeshIDMap.end()) // this is a new mesh data
	//	{
	//		meshId = msIdAllocator.Allocate();
	//		mSubMeshDatas.emplace_back(meshData, 1);
	//		mMeshIDMap[meshData.filename] = meshId;

	//		if (meshId == 0)
	//			mInstanceCountAreaSumTable.emplace_back(0);
	//		else
	//		{
	//			mInstanceCountAreaSumTable.emplace_back();
	//			mInstanceCountAreaSumTable[meshId] = mInstanceCountAreaSumTable[meshId - 1];
	//		}
	//	}
	//	else
	//	{
	//		meshId = mMeshIDMap[meshData.filename];
	//		mSubMeshDatas[meshId].second += 1;
	//		// update the area sum table. (each element behind the meshId plus one)
	//		if (meshId > 0)
	//			mInstanceCountAreaSumTable[meshId] = mInstanceCountAreaSumTable[meshId - 1] + (GetRefCount(meshId) <= 1 ? 0 : GetRefCount(meshId));
	//		for (UInt32 i = meshId + 1; i < mInstanceCountAreaSumTable.size(); ++i)
	//			mInstanceCountAreaSumTable[i] = mInstanceCountAreaSumTable[i - 1] + (GetRefCount(i) <= 1 ? 0 : GetRefCount(i));
	//	}
	//	return meshId;
	//}

	void MeshDataArchive::IncreaseRef(UInt32 meshId)
	{
		auto* pMeshData = GetData(meshId);
		SetData(*pMeshData); // just call SetData and it will do the increase ref counting
	}

	UInt32 MeshDataArchive::GetRefCount(UInt32 meshId) const
	{
		SG_ASSERT(meshId < mSubMeshDatas.size());
		return mSubMeshDatas[meshId].second;
	}

	const SubMeshData* MeshDataArchive::GetData(UInt32 meshId) const
	{
		SG_ASSERT(meshId < mSubMeshDatas.size());
		return &mSubMeshDatas[meshId].first;
	}

	const SubMeshData* MeshDataArchive::GetData(const string& filename) const
	{
		UInt32 meshId = GetMeshID(filename);
		return GetData(meshId);
	}

	UInt32 MeshDataArchive::GetMeshID(const string& filename) const
	{
		auto node = mMeshIDMap.find(filename);
		if (node != mMeshIDMap.end())
			return node->second;
		return UInt32(-1);
	}

	UInt32 MeshDataArchive::GetInstanceSumOffset(UInt32 meshId) const
	{
		SG_ASSERT(meshId < mInstanceCountAreaSumTable.size());
		return mInstanceCountAreaSumTable[meshId];
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
		for (auto& meshData : mSubMeshDatas)
			meshData.second = 0;
		for (UInt32 i = 0; i < mInstanceCountAreaSumTable.size(); ++i)
			mInstanceCountAreaSumTable[i] = 0;
	}

	void MeshDataArchive::LogDebugInfo()
	{
		SG_LOG_DEBUG("MeshDataArchive Debug Info:");
		UInt32 id = 0;
		for (auto& meshData : mSubMeshDatas)
		{
			SG_LOG_DEBUG("Mesh ID: %d", id);
			SG_LOG_DEBUG("    Filename: %s", meshData.first.filename.c_str());
			SG_LOG_DEBUG("    SubMeshName: %s", meshData.first.subMeshName.c_str());
			SG_LOG_DEBUG("    Ref Count: %d", meshData.second);
			++id;
		}

		SG_LOG_DEBUG("Area Sum Table:");
		string res = "[";
		for (UInt32 i = 0; i < mInstanceCountAreaSumTable.size() - 1; ++i)
		{
			res += eastl::to_string(mInstanceCountAreaSumTable[i]) + ", ";
		}
		res += eastl::to_string(mInstanceCountAreaSumTable[mInstanceCountAreaSumTable.size() - 1]);
		res += "]";
		SG_LOG_DEBUG("%s", res.c_str());
	}

	MeshDataArchive* MeshDataArchive::GetInstance()
	{
		static MeshDataArchive sInstance;
		return &sInstance;
	}

}