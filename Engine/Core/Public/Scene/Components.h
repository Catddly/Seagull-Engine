#pragma once

#include "Math/MathBasic.h"

#include "Archive/ResourceDefs.h"
#include "Archive/ResourceLoader.h"
#include "Archive/MeshDataArchive.h"
#include "Archive/MaterialAssetArchive.h"
#include "Render/MeshGenerate/MeshGenerator.h"
#include "Asset/Asset.h"
#include "Profile/Profile.h"

#include "TipECS/Registry.h"

#include "Stl/SmartPtr.h"

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
		explicit TagComponent(const char* n)
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
			glm::toMat4(Quaternion(glm::radians(comp.rotation))) *
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
		auto& subMesh = meshData.subMeshDatas.emplace_back();

		// if the mesh data already loaded, you don't need to set it again
		if (type == EGennerateMeshType::eGrid)
		{
			meshData.filename = "_generated_grid";
			subMesh.subMeshName = "_grid_0";
			subMesh.bIsProceduralMesh = true;
			if (!MeshDataArchive::GetInstance()->HaveMeshData(meshData.filename))
				MeshGenerator::GenGrid(subMesh.vertices, subMesh.indices);
			subMesh.aabb.minBound = { -0.51f, -0.01f, -0.51f };
			subMesh.aabb.maxBound = { 0.51f, 0.01f, 0.51f };
		}
		else if (type == EGennerateMeshType::eSkybox)
		{
			meshData.filename = "_generated_skybox";
			subMesh.subMeshName = "_skybox_0";
			subMesh.bIsProceduralMesh = true;
			if (!MeshDataArchive::GetInstance()->HaveMeshData(meshData.filename))
				MeshGenerator::GenSkybox(subMesh.vertices);
		}

		subMesh.filename = meshData.filename;

		comp.meshType = EMeshType::eUnknown;
		comp.meshId = MeshDataArchive::GetInstance()->SetData(meshData.subMeshDatas[0]);
	}

	SG_INLINE void LoadMesh(const char* filename, EMeshType type, MeshComponent& comp, ELoadMeshFlag flag = ELoadMeshFlag(0))
	{
		SG_PROFILE_FUNCTION();

		MeshData meshData = {};

		// if the mesh data already loaded, you don't need to set it again
		if (!MeshDataArchive::GetInstance()->HaveMeshData(filename))
		{
			MeshResourceLoader loader;
			if (!loader.LoadFromFile(filename, type, meshData, flag))
				SG_LOG_WARN("Mesh %s load failure!", filename);
		}

		comp.meshType = type;
		comp.meshId = MeshDataArchive::GetInstance()->SetData(meshData.subMeshDatas[0]);
	}

	SG_INLINE void LoadMesh(const char* filename, const char* subMeshName, EMeshType type, MeshComponent& comp, ELoadMeshFlag flag = ELoadMeshFlag(0))
	{
		SG_PROFILE_FUNCTION();

		MeshData meshData = {};

		// if the mesh data already loaded, you don't need to set it again
		if (!MeshDataArchive::GetInstance()->HaveMeshData(subMeshName))
		{
			MeshResourceLoader loader;
			if (!loader.LoadFromFile(filename, type, meshData, flag))
				SG_LOG_WARN("Mesh %s load failure!", filename);
			for (auto& subMeshData : meshData.subMeshDatas)
				MeshDataArchive::GetInstance()->SetData(subMeshData);
		}

		comp.meshType = type;
		comp.meshId = MeshDataArchive::GetInstance()->GetMeshID(subMeshName);
		MeshDataArchive::GetInstance()->IncreaseRef(comp.meshId);
	}

	SG_INLINE void CopyMesh(const MeshComponent& srcMesh, MeshComponent& dstMesh)
	{
		SG_PROFILE_FUNCTION();
		// just copy the reference id
		dstMesh.meshType = srcMesh.meshType;
		dstMesh.meshId = srcMesh.meshId;
		MeshDataArchive::GetInstance()->IncreaseRef(dstMesh.meshId);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MaterialComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct MaterialComponent
	{
		//Vector3f albedo = { 1.0f, 1.0f, 1.0f };
		//float    metallic = 0.1f;
		//float    roughness = 0.75f;
		//RefPtr<TextureAsset> albedoTex = nullptr;
		//RefPtr<TextureAsset> metallicTex = nullptr;
		//RefPtr<TextureAsset> roughnessTex = nullptr;
		//RefPtr<TextureAsset> normalTex = nullptr;
		//RefPtr<TextureAsset> AOTex = nullptr;
		//UInt32 materialAssetId = IDAllocator<UInt32>::INVALID_ID;

		WeakRefPtr<MaterialAsset> materialAsset;

		MaterialComponent() = default;
		MaterialComponent(const string& materialName, const Vector3f& c, float m, float r)
		{
			materialAsset = MaterialAssetArchive::GetInstance()->NewMaterialAsset(materialName, "");
			auto pMaterialAsset = materialAsset.lock();
			pMaterialAsset->SetAlbedo(c);
			pMaterialAsset->SetMetallic(m);
			pMaterialAsset->SetRoughness(r);
		}
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
		float    shadowMapScaleFactor = 1.0f;
		float    zNear = 1.0f;
		float    zFar = 1.0f;
		              
		DirectionalLightComponent() = default;
		DirectionalLightComponent(const Vector3f& c)
			:color(c)
		{}
	};

	SG_INLINE Vector3f CalcViewDirectionNormalized(const TransformComponent& trans)
	{
		SG_PROFILE_FUNCTION();

		Vector3f rotatedVec = Vector4f(SG_ENGINE_FRONT_VEC(), 0.0f) * glm::toMat4(Quaternion(glm::radians(trans.rotation)));
		return glm::normalize(rotatedVec);

		//Vector3f direction = -trans.position;
		//return glm::normalize(direction);
	}

	SG_INLINE Matrix4f CalcDirectionalLightViewProj(const TransformComponent& trans, const Vector3f& viewDirection, const Vector3f& cameraPos, 
		float aspectRatio, float shadowMapScaleFactor, float zNear, float zFar)
	{
		SG_PROFILE_FUNCTION();

		return BuildOrthographicMatrix(-shadowMapScaleFactor * aspectRatio, shadowMapScaleFactor * aspectRatio, -shadowMapScaleFactor, shadowMapScaleFactor, zNear, zFar) *
			BuildViewMatrixCenter(cameraPos - (viewDirection * trans.position.y), cameraPos, SG_ENGINE_UP_VEC());
		//BuildViewMatrixDirection(trans.position, CalcViewDirectionNormalized(trans), SG_ENGINE_UP_VEC());

		//return BuildPerspectiveMatrix(glm::radians(45.0f), 1.0f, 1.0f, 300.0f) *
		//	BuildViewMatrixDirection(trans.position, CalcViewDirectionNormalized(trans), SG_ENGINE_UP_VEC());
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// CameraComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: add a scripts component to control the camera
	struct CameraComponent
	{
		ECameraType     type = ECameraType::eFirstPerson;
		RefPtr<ICamera> pCamera;

		CameraComponent() = default;
	};

#define COMPONENTS(F, END) \
F(TagComponent) \
F(MeshComponent) \
F(PointLightComponent) \
F(DirectionalLightComponent) \
F(MaterialComponent) \
F(CameraComponent) \
END(TransformComponent)

	struct LightTag {};

#define TAGS(F, END) \
END(LightTag)

	using Signature1 = TipECS::Signature<TransformComponent, MeshComponent, MaterialComponent>;
	using Signature2 = TipECS::Signature<TagComponent, PointLightComponent>;
	using Signature3 = TipECS::Signature<LightTag>;
	using Signature4 = TipECS::Signature<TransformComponent, CameraComponent>;

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