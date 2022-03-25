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
		SG_CORE_API explicit Mesh(const char* name);
		SG_CORE_API explicit Mesh(const char* name, EGennerateMeshType type);
		SG_CORE_API explicit Mesh(const char* name, const char* objectName, EMeshType type);
		SG_CORE_API Mesh(const char* name, const vector<float>& vertices, const vector<UInt32>& indices);
		~Mesh() = default;

		Mesh(const Mesh&) = default;
		Mesh(Mesh&&) = default;

		void Copy(const Mesh& mesh);

		const string&   GetName() const noexcept { return mName; }
		const EMeshType GetType() const noexcept { return mType; }

		const UInt32 GetMeshID()     const { return mMeshId; }
		const UInt32 GetInstanceID() const { return mInstanceId; }
		const UInt32 GetObjectID()   const { return mObjectId; }
	private:
		static UInt32 NewID();
	private:
		string    mName;
		EMeshType mType;
		UInt32    mMeshId = UInt32(-1);   //! Used to reference mesh data.
		UInt32    mInstanceId = 0;        //! Used to identify instance, if this mesh do not have instance, the default is 0. Because one object can be seen as one instance.
		UInt32    mObjectId = UInt32(-1); //! Used as UUID(or GUID) in Object System.

		// temporary (should use a simple id allocation system)
		static UInt32 msCurrId;
	};

}