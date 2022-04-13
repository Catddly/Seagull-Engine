#pragma once

#include "Math/MathBasic.h"

#include "Scene/Mesh/MeshDataArchive.h"
#include "Render/MeshGenerate/MeshGenerator.h"
#include "Archive/ResourceLoader/RenderResourceLoader.h"
#include "Archive/ResourceLoader/ResourceDefs.h"

#include "TipECS/Registry.h"

namespace SG
{

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// TagComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct TagComponent
	{
		string name;
		bool   bDirty = true;

		TagComponent() = default;
		TagComponent(const char* n)
			: name(n)
		{}
		TagComponent(const string& str)
			: name(str)
		{}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// TransformComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct TransformComponent
	{
		Vector3f position = Vector3f(0.0f);
		Vector3f scale = Vector3f(1.0f);
		Vector3f rotation = Vector3f(0.0f);

		TransformComponent() = default;
		TransformComponent(const Vector3f& pos, const Vector3f& s, const Vector3f& rot)
			:position(pos), scale(s), rotation(rot)
		{}
	};

	SG_INLINE Matrix4f GetTransform(const TransformComponent& comp)
	{
		return glm::translate(Matrix4f(1.0f), comp.position) *
			glm::toMat4(Quternion(glm::radians(comp.rotation))) *
			glm::scale(Matrix4f(1.0f), comp.scale);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MeshComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct MeshComponent
	{
		EMeshType meshType   = EMeshType::eUnknown;
		UInt32    meshId     = UInt32(-1); //! Used to reference mesh data.
		UInt32    instanceId = 0;          //! Used to identify instance, if this mesh do not have instance, the default is 0. Because one object can be seen as one instance.
		UInt32    objectId   = UInt32(-1); //! Used as UUID(or GUID) in Object System.

		MeshComponent() = default;
		MeshComponent(EMeshType t, UInt32 mId, UInt32 iId)
			:meshType(t), meshId(mId), instanceId(iId)
		{}
	};

	SG_INLINE void LoadMesh(EGennerateMeshType type, MeshComponent& comp)
	{
		MeshData meshData = {};
		if (type == EGennerateMeshType::eGrid)
			MeshGenerator::GenGrid(meshData.vertices, meshData.indices);
		else if (type == EGennerateMeshType::eSkybox)
			MeshGenerator::GenSkybox(meshData.vertices);
		comp.meshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	SG_INLINE void LoadMesh(const char* filename, EMeshType type, MeshComponent& comp)
	{
		string fullName = filename;
		fullName += MeshTypeToExtString(type);

		MeshData meshData = {};
		MeshResourceLoader loader;
		if (!loader.LoadFromFile(fullName.c_str(), meshData.vertices, meshData.indices))
			SG_LOG_WARN("Mesh %s load failure!", fullName);
		comp.meshType = type;
		comp.meshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	SG_INLINE void CopyMesh(const MeshComponent& srcMesh, MeshComponent& dstMesh)
	{
		dstMesh.meshType = srcMesh.meshType;
		dstMesh.meshId = srcMesh.meshId;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MaterialComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct MaterialComponent
	{
		Vector3f color = { 1.0f, 1.0f, 1.0f };
		float    metallic = 0.7f;
		float    roughness = 0.35f;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// PointLightComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct PointLightComponent
	{
		Vector3f color;
		float    radius;

		PointLightComponent() = default;
		PointLightComponent(const Vector3f c, float r)
			:color(c), radius(r)
		{}
	};

#define COMPONENTS(F, END) \
F(TagComponent) \
F(MeshComponent) \
F(PointLightComponent) \
F(MaterialComponent) \
END(TransformComponent)

	using Signature1 = TipECS::Signature<TransformComponent, MeshComponent, MaterialComponent>;
	using Signature2 = TipECS::Signature<TagComponent, PointLightComponent>;

#define SIGNATURES(F, END) \
F(Signature1) \
END(Signature2)

#define MACRO_EXPAND(NAME) NAME,
#define MACRO_EXPAND_END(NAME) NAME
	using SGComponentList = TipECS::ComponentList<COMPONENTS(MACRO_EXPAND, MACRO_EXPAND_END)>;
	using SGTagList = TipECS::TagList<>;
	using SGSignatureList = TipECS::SignatureList<SIGNATURES(MACRO_EXPAND, MACRO_EXPAND_END)>;
#undef MACRO_EXPAND
#undef MACRO_EXPAND_END

	using SGECSSetting = TipECS::Setting<SGComponentList, SGTagList, SGSignatureList>;

}