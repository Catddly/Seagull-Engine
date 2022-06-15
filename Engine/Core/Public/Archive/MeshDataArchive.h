#pragma once

#include "Archive/IDAllocator.h"
#include "Math/BoundingBox.h"

#include "Stl/vector.h"
#include "Stl/string.h"
#include "eastl/unordered_map.h"

namespace SG
{

	struct SubMeshData
	{
		vector<float>  vertices = {};
		vector<UInt32> indices = {};
		AABB   aabb; // this aabb is the mesh aabb, not the world space aabb.
		string subMeshName = "";
		string filename = ""; //! Ref pointer to its parent.

		UInt32 materialAssetId = IDAllocator<UInt32>::INVALID_ID; //! If this sub-mesh have embedded texture, it will contain an valid id.
		bool   bIsProceduralMesh = false;
	};

	struct MeshData
	{
		vector<SubMeshData> subMeshDatas;
		string filename = "";
	};

	class MeshDataArchive
	{
	public:
		SG_CORE_API UInt32 SetData(const SubMeshData& subMeshData);
		//SG_CORE_API UInt32 SetData(const MeshData& meshData);

		SG_CORE_API void IncreaseRef(UInt32 meshId);

		SG_CORE_API UInt32 GetRefCount(UInt32 meshId) const;

		//SG_CORE_API const MeshData* GetData(const string& filename) const;
		//SG_CORE_API const MeshData* GetData(UInt32 meshId) const;

		SG_CORE_API SubMeshData* GetData(const string& filename);
		SG_CORE_API const SubMeshData* GetData(const string& filename) const;
		SG_CORE_API SubMeshData* GetData(UInt32 meshId);
		SG_CORE_API const SubMeshData* GetData(UInt32 meshId) const;

		SG_CORE_API UInt32 GetMeshID(const string& filename) const;

		SG_CORE_API UInt32 GetNumMeshData() const { return static_cast<UInt32>(mSubMeshDatas.size()); }

		SG_CORE_API UInt32 GetInstanceSumOffset(UInt32 meshId) const;

		SG_CORE_API bool HaveInstance(UInt32 meshId) const;
		SG_CORE_API bool HaveMeshData(const string& filename);

		SG_CORE_API void Reset();

		SG_CORE_API void LogDebugInfo();

		SG_CORE_API static MeshDataArchive* GetInstance();
	private:
		MeshDataArchive() = default;
	private:
		eastl::vector<eastl::pair<SubMeshData, UInt32>> mSubMeshDatas; // meshId -> pair<SubMeshData, RefCount(Instance Count)>
		eastl::unordered_map<string, UInt32> mMeshIDMap; // filename -> meshId

		eastl::vector<UInt32> mInstanceCountAreaSumTable;

		static IDAllocator<UInt32> msIdAllocator;
	};

}