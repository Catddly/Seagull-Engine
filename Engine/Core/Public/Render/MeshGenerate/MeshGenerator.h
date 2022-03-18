#pragma once

#include "Stl/vector.h"

namespace SG
{

	enum class EGennerateMeshType
	{
		eGrid = 0,
		eSkybox,
	};

	class MeshGenerator
	{
	public:
		static void GenGrid(vector<float>& outVertices, vector<UInt32>& outIndices);
		static void GenSkybox(vector<float>& outVertices);
	};

}