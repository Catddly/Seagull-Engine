#pragma once

#include "Stl/vector.h"
#include "Stl/string.h"
#include "eastl/unordered_map.h"

namespace SG
{

	struct MeshData
	{
		vector<float>  vertices;
		vector<UInt32> indices;
	};

	class MeshDataArchive
	{
	public:
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