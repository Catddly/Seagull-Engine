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

}