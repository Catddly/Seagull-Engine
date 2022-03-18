#include "StdAfx.h"
#include "Render/MeshGenerate/MeshGenerator.h"

namespace SG
{

	void MeshGenerator::GenGrid(vector<float>& outVertices, vector<UInt32>& outIndices)
	{
		// [0] vertex
		outVertices.push_back(-0.5f);
		outVertices.push_back(0.0f);
		outVertices.push_back(-0.5f);
		// [0] normal
		outVertices.push_back(0.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(0.0f);

		// [1] vertex
		outVertices.push_back(0.5f);
		outVertices.push_back(0.0f);
		outVertices.push_back(-0.5f);
		// [1] normal
		outVertices.push_back(0.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(0.0f);

		// [2] vertex
		outVertices.push_back(0.5f);
		outVertices.push_back(0.0f);
		outVertices.push_back(0.5f);
		// [2] normal
		outVertices.push_back(0.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(0.0f);

		// [3] vertex
		outVertices.push_back(-0.5f);
		outVertices.push_back(0.0f);
		outVertices.push_back(0.5f);
		// [3] normal
		outVertices.push_back(0.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(0.0f);
		
		// [0] triangle
		outIndices.push_back(0);
		outIndices.push_back(2);
		outIndices.push_back(1);

		// [1] triangle
		outIndices.push_back(0);
		outIndices.push_back(3);
		outIndices.push_back(2);
	}

	void MeshGenerator::GenSkybox(vector<float>& outVertices)
	{
		// [0] triangle (-Z)
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		// [1] triangle (-Z)
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);

		// [2] triangle (+X)
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		// [3] triangle (+X)
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		// [4] triangle (-Z)
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		// [5] triangle (-Z)
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		// [6] triangle (-X)
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		// [7] triangle (-X)
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		// [8] triangle (+Y)
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		// [9] triangle (+Y)
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		outVertices.push_back(1.0f);
		// [10] triangle (-Y)
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);

		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);
		// [11] triangle (-Y)
		outVertices.push_back(1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(1.0f);

		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
		outVertices.push_back(-1.0f);
	}

}