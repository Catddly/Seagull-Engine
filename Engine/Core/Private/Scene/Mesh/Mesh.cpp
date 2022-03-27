#include "StdAfx.h"
#include "Scene/Mesh/Mesh.h"

#include "Archive/ResourceLoader/RenderResourceLoader.h"
#include "Scene/Mesh/MeshDataArchive.h"

namespace SG
{

	UInt32 Mesh::NewID()
	{
		return msCurrId++;
	}

	UInt32 Mesh::msCurrId = 1;

	Mesh::Mesh(const char* name)
		:mName(name), mType(EMeshType::eUnknown), mObjectId(NewID()), mInstanceId(0)
	{
	}

	Mesh::Mesh(const char* name, EGennerateMeshType type)
		:mName(name), mType(EMeshType::eUnknown), mObjectId(NewID()), mInstanceId(0)
	{
		MeshData meshData = {};
		if (type == EGennerateMeshType::eGrid)
			MeshGenerator::GenGrid(meshData.vertices, meshData.indices);
		else if (type == EGennerateMeshType::eSkybox)
			MeshGenerator::GenSkybox(meshData.vertices);
		mMeshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	Mesh::Mesh(const char* name, const char* objectName, EMeshType type)
		:mName(name), mType(type), mObjectId(NewID()), mInstanceId(0)
	{
		MeshResourceLoader loader;
		string fullName = objectName;
		fullName += MeshTypeToExtString(mType);

		MeshData meshData = {};
		if (!loader.LoadFromFile(fullName.c_str(), meshData.vertices, meshData.indices))
			SG_LOG_WARN("Mesh %s load failure!", fullName);
		mMeshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	Mesh::Mesh(const char* name, const vector<float>& vertices, const vector<UInt32>& indices)
		:mName(name), mType(EMeshType::eUnknown), mObjectId(NewID()), mInstanceId(0)
	{
		MeshData meshData = {};
		meshData.vertices = vertices;
		meshData.indices = indices;
		mMeshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	void Mesh::Copy(const Mesh& mesh)
	{
		this->mType = mesh.mType;
		this->mMeshId = mesh.mMeshId;
	}

}