#pragma once

#include "Archive/IDAllocator.h"

#include "Stl/vector.h"
#include "Stl/string.h"
#include "eastl/unordered_map.h"

namespace SG
{

	struct SubMeshData
	{
		vector<float>  vertices = {};
		vector<UInt32> indices = {};
		string subMeshName = "";
		string filename = ""; //! Ref pointer to its parent.
	};

	struct MeshData
	{
		vector<SubMeshData> subMeshDatas;
		string filename = "";
		bool   bIsProceduralMesh = false;
	};

	class MeshDataArchive
	{
	public:
		SG_CORE_API UInt32 SetData(const MeshData& meshData);

		SG_CORE_API void IncreaseRef(UInt32 meshId);

		SG_CORE_API UInt32 GetRefCount(UInt32 meshId) const;
		SG_CORE_API const MeshData* GetData(UInt32 meshId) const;
		SG_CORE_API UInt32 GetMeshID(const string& filename) const;

		SG_CORE_API UInt32 GetNumMeshData() const { return static_cast<UInt32>(mMeshDatas.size()); }

		SG_CORE_API UInt32 GetInstanceSumOffset(UInt32 meshId) const;

		SG_CORE_API bool HaveInstance(UInt32 meshId) const;
		SG_CORE_API bool HaveMeshData(const string& filename);

		SG_CORE_API void Reset();

		SG_CORE_API void LogDebugInfo();

		SG_CORE_API static MeshDataArchive* GetInstance();
	private:
		MeshDataArchive() = default;
	private:
		eastl::vector<eastl::pair<MeshData, UInt32>> mMeshDatas; // meshId -> pair<MeshData, RefCount(Instance Count)>
		eastl::unordered_map<string, UInt32> mMeshIDMap; // filename -> meshId

		eastl::vector<UInt32> mInstanceCountAreaSumTable;

		static IDAllocator<UInt32> msIdAllocator;
	};

}