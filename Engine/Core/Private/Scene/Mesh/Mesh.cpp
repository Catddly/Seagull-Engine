#include "StdAfx.h"
#include "Scene/Mesh/Mesh.h"

#include "Archive/ResourceLoader/RenderResourceLoader.h"

namespace SG
{

	Mesh::Mesh(const char* name, EGennerateMeshType type)
		:mName(name), mType(EMeshType::eUnknown)
	{
		if (type == EGennerateMeshType::eGrid)
			MeshGenerator::GenGrid(mVertices, mIndices);
		else if (type == EGennerateMeshType::eSkybox)
			MeshGenerator::GenSkybox(mVertices);
	}

	Mesh::Mesh(const char* name, EMeshType type)
		:mName(name), mType(type)
	{
		MeshResourceLoader loader;
		string fullName = mName;
		fullName += MeshTypeToExtString(mType);

		if (!loader.LoadFromFile(fullName.c_str(), mVertices, mIndices))
			SG_LOG_WARN("Mesh %s load failure!", fullName);
	}

	Mesh::Mesh(const char* name, const vector<float>& vertices, const vector<UInt32>& indices)
		:mName(name), mVertices(vertices), mIndices(indices), mType(EMeshType::eUnknown)
	{
	}

}