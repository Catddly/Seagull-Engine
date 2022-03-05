#include "StdAfx.h"
#include "Scene/Mesh/Mesh.h"

#include "Archive/ResourceLoader/RenderResourceLoader.h"

namespace SG
{

	Mesh::Mesh(const char* name, EMeshType type)
		:mName(name), mType(type)
	{
		MeshResourceLoader loader;
		string fullName = mName;
		fullName += MeshTypeToExtString(mType);

		if (!loader.LoadFromFile(fullName.c_str(), mVertices, mIndices))
			SG_LOG_WARN("Mesh %s load failure!", fullName);
	}

}