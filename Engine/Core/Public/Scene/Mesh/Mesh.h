#pragma once

#include "Render/MeshGenerate/MeshGenerator.h"
#include "Archive/ResourceLoader/ResourceDefs.h"
#include "Scene/Components.h"

#include "Stl/vector.h"

namespace SG
{

	class Mesh : public TransformComponent, public MaterialComponent
	{
	public:
		explicit Mesh(const char* name);
		explicit Mesh(const char* name, EGennerateMeshType type);
		explicit Mesh(const char* name, EMeshType type);
		Mesh(const char* name, const vector<float>& vertices, const vector<UInt32>& indices);
		~Mesh() = default;

		void Copy(const Mesh& mesh);

		const string&   GetName() const noexcept { return mName; }
		const EMeshType GetType() const noexcept { return mType; }

		const UInt32 GetMeshID() const { return mMeshId; }
		const UInt32 GetObjectID() const { return mObjectId; }
	private:
		static UInt32 NewID();
	private:
		string         mName;
		EMeshType      mType;
		UInt32         mMeshId = UInt32(-1);
		UInt32         mObjectId = UInt32(-1);

		// temporary (should use a simple id allocation system)
		static UInt32 msCurrId;
	};

}