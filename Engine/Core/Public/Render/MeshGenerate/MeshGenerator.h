#pragma once

#include "Stl/vector.h"

namespace SG
{

	enum class EGennerateMeshType
	{
		eGrid = 0,
	};

	class MeshGenerator
	{
	public:
		static void GenGrid(vector<float>& outVertices, vector<UInt32>& outIndices);
	};

}