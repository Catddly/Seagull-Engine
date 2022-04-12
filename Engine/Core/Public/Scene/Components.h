#pragma once

#include "Math/MathBasic.h"

#include "Render/MeshGenerate/MeshGenerator.h"
#include "Archive/ResourceLoader/ResourceDefs.h"

#include "TipECS/Registry.h"

namespace SG
{

	class TransformComponent
	{
	public:
		TransformComponent()
			:mPosition(0.0f, 0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f), mRotation(0.0f, 0.0f, 0.0f)
		{}

		SG_INLINE void SetPosition(const Vector3f& pos) { mPosition = pos; }
		SG_INLINE Vector3f GetPosition() const { return mPosition; };

		SG_INLINE void SetScale(const Vector3f& scale) { mScale = scale; }
		SG_INLINE Vector3f GetScale()    const { return mScale; };

		SG_INLINE void SetRotation(const Vector3f& rotation) { mRotation = rotation; }
		SG_INLINE Vector3f GetRotation() const { return mRotation; };

		SG_INLINE Matrix4f GetTransform() const
		{
			return glm::translate(Matrix4f(1.0f), mPosition) *
				glm::toMat4(Quternion(glm::radians(mRotation))) *
				glm::scale(Matrix4f(1.0f), mScale);
		}
	private:
		Vector3f mPosition;
		Vector3f mScale;
		Vector3f mRotation;
	};

	//struct TransformComponent
	//{
	//	Vector3f position = Vector3f(0.0f);
	//	Vector3f scale = Vector3f(1.0f);
	//	Vector3f rotation = Vector3f(0.0f);
	//};

	//SG_INLINE Matrix4f GetTransform(const TransformComponent& comp)
	//{
	//	return glm::translate(Matrix4f(1.0f), comp.position) *
	//		glm::toMat4(Quternion(glm::radians(comp.rotation))) *
	//		glm::scale(Matrix4f(1.0f), comp.scale);
	//}

	class MaterialComponent
	{
	public:
		SG_INLINE void     SetColor(const Vector3f& color) { mColor = color; }
		SG_INLINE Vector3f GetColor() const { return mColor; }

		SG_INLINE void  SetRoughness(float roughness) { mRoughness = roughness; }
		SG_INLINE float GetRoughness() const { return mRoughness; }

		SG_INLINE void  SetMetallic(float metallic) { mMetallic = metallic; }
		SG_INLINE float GetMetallic()  const { return mMetallic; }
	private:
		Vector3f mColor = { 1.0f, 1.0f, 1.0f };
		float    mMetallic = 0.7f;
		float    mRoughness = 0.35f;
	};

	//UInt32 NewObjectID();

	struct MeshComponent
	{
		EMeshType meshType   = EMeshType::eUnknown;
		UInt32    meshId     = UInt32(-1); //! Used to reference mesh data.
		UInt32    instanceId = 0;          //! Used to identify instance, if this mesh do not have instance, the default is 0. Because one object can be seen as one instance.
		UInt32    objectId   = UInt32(-1); //! Used as UUID(or GUID) in Object System.
	};

	//SG_INLINE void LoadMesh(EGennerateMeshType type, MeshComponent& comp)
	//{
	//	comp.objectId = NewObjectID();
	//	MeshData meshData = {};
	//	if (type == EGennerateMeshType::eGrid)
	//		MeshGenerator::GenGrid(meshData.vertices, meshData.indices);
	//	else if (type == EGennerateMeshType::eSkybox)
	//		MeshGenerator::GenSkybox(meshData.vertices);
	//	comp.meshId = MeshDataArchive::GetInstance()->SetData(meshData);
	//}

	//SG_INLINE void LoadMesh(const char* filename, EMeshType type, MeshComponent& comp)
	//{
	//	comp.objectId = NewObjectID();
	//	string fullName = filename;
	//	fullName += MeshTypeToExtString(type);

	//	MeshData meshData = {};
	//	MeshResourceLoader loader;
	//	if (!loader.LoadFromFile(fullName.c_str(), meshData.vertices, meshData.indices))
	//		SG_LOG_WARN("Mesh %s load failure!", fullName);
	//	comp.meshType = type;
	//	comp.meshId = MeshDataArchive::GetInstance()->SetData(meshData);
	//}

	//SG_INLINE void CopyMesh(const MeshComponent& srcMesh, MeshComponent& dstMesh)
	//{
	//	dstMesh.meshType = srcMesh.meshType;
	//	dstMesh.meshId = srcMesh.meshId;
	//}

	//struct MaterialComponent
	//{
	//	Vector3f color = { 1.0f, 1.0f, 1.0f };
	//	float    metallic = 0.7f;
	//	float    roughness = 0.35f;
	//};

	struct TagComponent
	{
		string name;

		TagComponent() = default;
		TagComponent(const char* n) 
			: name(n)
		{}
	};

#define COMPONENTS(F, END) \
F(TagComponent) \
F(MeshComponent) \
F(TransformComponent) \
END(MaterialComponent)

#define MACRO_EXPAND(NAME) NAME,
#define MACRO_EXPAND_END(NAME) NAME
	using SGComponentList = TipECS::ComponentList<COMPONENTS(MACRO_EXPAND, MACRO_EXPAND_END)>;
#undef MACRO_EXPAND
#undef MACRO_EXPAND_END
	using SGTagList = TipECS::TagList<>;

	using TMSignature = TipECS::Signature<TransformComponent, MaterialComponent>;
	using SGSignatureList = TipECS::SignatureList<TMSignature>;

	using SGECSSetting = TipECS::Setting<SGComponentList, SGTagList, SGSignatureList>;

}