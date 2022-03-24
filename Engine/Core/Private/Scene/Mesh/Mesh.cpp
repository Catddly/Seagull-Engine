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
		:mName(name), mType(EMeshType::eUnknown), mObjectId(NewID())
	{
	}

	Mesh::Mesh(const char* name, EGennerateMeshType type)
		:mName(name), mType(EMeshType::eUnknown), mObjectId(NewID())
	{
		MeshData meshData = {};
		if (type == EGennerateMeshType::eGrid)
			MeshGenerator::GenGrid(meshData.vertices, meshData.indices);
		else if (type == EGennerateMeshType::eSkybox)
			MeshGenerator::GenSkybox(meshData.vertices);
		mMeshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	Mesh::Mesh(const char* name, EMeshType type)
		:mName(name), mType(type), mObjectId(NewID())
	{
		MeshResourceLoader loader;
		string fullName = mName;
		fullName += MeshTypeToExtString(mType);

		MeshData meshData = {};
		if (!loader.LoadFromFile(fullName.c_str(), meshData.vertices, meshData.indices))
			SG_LOG_WARN("Mesh %s load failure!", fullName);
		mMeshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	Mesh::Mesh(const char* name, const vector<float>& vertices, const vector<UInt32>& indices)
		:mName(name), mType(EMeshType::eUnknown), mObjectId(NewID())
	{
		MeshData meshData = {};
		meshData.vertices = vertices;
		meshData.indices = indices;
		mMeshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	void Mesh::Copy(const Mesh& mesh)
	{
		this->mMeshId = mesh.mMeshId;
		this->mType = mesh.mType;
	}

	//Mesh::Mesh(const Mesh& mesh)
	//	:mObjectId(NewID())
	//{
	//	mType = mesh.mType;
	//	mMeshId = mesh.mMeshId;
	//}

	//Mesh::Mesh(Mesh&& mesh)
	//{
	//	this->mName = eastl::move(mesh.mName);
	//	this->mType = eastl::move(mesh.mType);
	//	this->mMeshId = eastl::move(mesh.mMeshId);
	//	this->mObjectId = eastl::move(mesh.mObjectId);
	//}

	//Mesh& Mesh::operator=(const Mesh& mesh)
	//{
	//	mType = mesh.mType;
	//	mMeshId = mesh.mMeshId;
	//	return *this;
	//}

	//Mesh& Mesh::operator=(Mesh&& mesh)
	//{
	//	this->mName = eastl::move(mesh.mName);
	//	this->mType = eastl::move(mesh.mType);
	//	this->mMeshId = eastl::move(mesh.mMeshId);
	//	this->mObjectId = eastl::move(mesh.mObjectId);
	//	return *this;
	//}

}