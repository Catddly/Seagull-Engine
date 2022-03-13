#pragma once

#include "Stl/vector.h"

namespace SG
{

	class MeshGenerator
	{
	public:
		static void GenGrid(vector<float>& outVertices, vector<UInt32>& outIndices);
	};

}