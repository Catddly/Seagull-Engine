#pragma once

#include "Math/MathBasic.h"

#include "Scene/Mesh/MeshDataArchive.h"
#include "Scene/ResourceLoader/RenderResourceLoader.h"
#include "Scene/ResourceLoader/ResourceDefs.h"
#include "Render/MeshGenerate/MeshGenerator.h"
#include "Profile/Profile.h"

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
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

		MeshData meshData = {};
		if (type == EGennerateMeshType::eGrid)
			MeshGenerator::GenGrid(meshData.vertices, meshData.indices);
		else if (type == EGennerateMeshType::eSkybox)
			MeshGenerator::GenSkybox(meshData.vertices);
		comp.meshId = MeshDataArchive::GetInstance()->SetData(meshData);
	}

	SG_INLINE void LoadMesh(const char* filename, EMeshType type, MeshComponent& comp)
	{
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

		dstMesh.meshType = srcMesh.meshType;
		dstMesh.meshId = srcMesh.meshId;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MaterialComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct MaterialComponent
	{
		Vector3f albedo = { 1.0f, 1.0f, 1.0f };
		// TODO: temporary: should be replace by the actual handle
		const string& albedoTexture = "";

		float    metallic = 0.7f;
		float    roughness = 0.35f;

		MaterialComponent() = default;
		MaterialComponent(const Vector3f& c, float m, float r)
			:albedo(c), metallic(m), roughness(r)
		{}
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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// DirectionalLightComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct DirectionalLightComponent
	{
		Vector3f color = Vector3f(1.0f);

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const Vector3f& c)
			:color(c)
		{}
	};

	SG_INLINE Vector3f CalcViewDirectionNormalized(const TransformComponent& trans)
	{
		SG_PROFILE_FUNCTION();

		Vector3f rotatedVec = Vector4f(SG_ENGINE_FRONT_VEC(), 0.0f) * glm::toMat4(Quternion(glm::radians(trans.rotation)));
		return glm::normalize(rotatedVec);
	}

	SG_INLINE Matrix4f CalcDirectionalLightViewProj(const TransformComponent& trans)
	{
		SG_PROFILE_FUNCTION();

		return BuildOrthographicMatrix(-10.0f, 10.0f, -10.0f, 10.0f, 0.0001f, 200.0f) *
			BuildViewMatrixDirection(trans.position, CalcViewDirectionNormalized(trans), SG_ENGINE_UP_VEC());
	}

#define COMPONENTS(F, END) \
F(TagComponent) \
F(MeshComponent) \
F(PointLightComponent) \
F(DirectionalLightComponent) \
F(MaterialComponent) \
END(TransformComponent)

	struct LightTag {};

#define TAGS(F, END) \
END(LightTag)

	using Signature1 = TipECS::Signature<TransformComponent, MeshComponent, MaterialComponent>;
	using Signature2 = TipECS::Signature<TagComponent, PointLightComponent>;
	using Signature3 = TipECS::Signature<LightTag>;

#define SIGNATURES(F, END) \
F(Signature1) \
F(Signature3) \
END(Signature2)

#define MACRO_EXPAND(NAME) NAME,
#define MACRO_EXPAND_END(NAME) NAME
	using SGComponentList = TipECS::ComponentList<COMPONENTS(MACRO_EXPAND, MACRO_EXPAND_END)>;
	using SGTagList = TipECS::TagList<TAGS(MACRO_EXPAND, MACRO_EXPAND_END)>;
	using SGSignatureList = TipECS::SignatureList<SIGNATURES(MACRO_EXPAND, MACRO_EXPAND_END)>;
#undef MACRO_EXPAND
#undef MACRO_EXPAND_END

	using SGECSSetting = TipECS::Setting<SGComponentList, SGTagList, SGSignatureList>;

#undef COMPONENTS
#undef TAGS
#undef SIGNATURES

}