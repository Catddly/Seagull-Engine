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
		//! Generated Grid.
		//! Vertices: [0] vertex pos, [1] normal, [2] uv, [3] tangent.
		//! Indices: contain.
		static void GenGrid(vector<float>& outVertices, vector<UInt32>& outIndices);
		//! Generated Skybox.
		//! Vertex: [0] vertex pos.
		//! No indices. Vertices only.
		static void GenSkybox(vector<float>& outVertices);
	};

}