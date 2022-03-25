#pragma once

#include "Stl/vector.h"
#include "Stl/string.h"
#include "eastl/unordered_map.h"

namespace SG
{

	struct MeshData
	{
		// TODO: use reference counted system.
		UInt32 numReference = 0; // put referenceCnt in here is dangerous, because user can modified it.
		vector<float>  vertices;
		vector<UInt32> indices;
	};

	class MeshDataArchive
	{
	public:
		SG_CORE_API UInt32 AddRef(UInt32 meshId);
		SG_CORE_API UInt32 DecreaseRef(UInt32 meshId);

		SG_CORE_API UInt32 SetData(const MeshData& meshData);
		SG_CORE_API const MeshData* GetData(UInt32 meshId) const;

		SG_CORE_API static MeshDataArchive* GetInstance();
	private:
		MeshDataArchive() = default;
	private:
		eastl::unordered_map<UInt32, MeshData> mMeshDatas;

		static UInt32 msCurrKey;
	};

}