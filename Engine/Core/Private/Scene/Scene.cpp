#include "StdAfx.h"
#include "Scene/Scene.h"

#include "Archive/IDAllocator.h"

#include "Platform/OS.h"
#include "Scene/Camera/FirstPersonCamera.h"
#include "Scene/Mesh/MeshDataArchive.h"
#include "Scene/Mesh/TextureAssetArchive.h"
#include "Profile/Profile.h"

namespace SG
{

	namespace // anonymous namespace
	{
		IDAllocator<UInt32> gObjectIdAllocator;
	}

	static void _OnMeshComponentAdded(const Scene::Entity& entity, MeshComponent& comp)
	{
		comp.objectId = gObjectIdAllocator.Allocate();
	}

	static void _OnMeshComponentRemoved(const Scene::Entity& entity, MeshComponent& comp)
	{
		gObjectIdAllocator.Restore(comp.objectId);
	}

	static void _SerializeEntity(Scene::Entity& entity, json& node)
	{
		node["EntityID"] = "2468517968452275"; // TEMPORARY

		auto& tagNode = node["TagComponent"];
		{
			auto& tag = entity.GetComponent<TagComponent>();
			tagNode["Name"] = tag.name.c_str();
		}

		auto& transNode = node["TransformComponent"];
		{
			auto& trans = entity.GetComponent<TransformComponent>();
			transNode["Position"] = trans.position;
			transNode["Rotation"] = trans.rotation;
			transNode["Scale"] = trans.scale;
		}

		if (entity.HasComponent<MeshComponent>())
		{
			auto& meshNode = node["MeshComponent"];
			{
				auto& mesh = entity.GetComponent<MeshComponent>();
				auto* pMeshData = MeshDataArchive::GetInstance()->GetData(mesh.meshId);
				meshNode["Filename"] = pMeshData->filename.c_str();
				meshNode["IsProceduralMesh"] = pMeshData->bIsProceduralMesh;
			}
		}

		if (entity.HasComponent<MaterialComponent>())
		{
			auto& matNode = node["MaterialComponent"];
			{
				auto& mat = entity.GetComponent<MaterialComponent>();

				matNode["Albedo"] = mat.albedo;
				matNode["Metallic"] = mat.metallic;
				matNode["Roughness"] = mat.roughness;
				if (mat.albedoTex)
				{
					auto& assetNode = matNode["AlbedoTextureAsset"];
					mat.albedoTex->Serialize(assetNode);
				}
				if (mat.metallicTex)
				{
					auto& assetNode = matNode["MetallicTextureAsset"];
					mat.metallicTex->Serialize(assetNode);
				}
				if (mat.roughnessTex)
				{
					auto& assetNode = matNode["RoughnessTextureAsset"];
					mat.roughnessTex->Serialize(assetNode);
				}
				if (mat.normalTex)
				{
					auto& assetNode = matNode["NormalTextureAsset"];
					mat.normalTex->Serialize(assetNode);
				}
				if (mat.AOTex)
				{
					auto& assetNode = matNode["AOTextureAsset"];
					mat.AOTex->Serialize(assetNode);
				}

			}
		}

		if (entity.HasComponent<PointLightComponent>())
		{
			auto& lightNode = node["PointLightComponent"];
			{
				auto& light = entity.GetComponent<PointLightComponent>();
				lightNode["Color"] = light.color;
				lightNode["Radius"] = light.radius;
			}
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			auto& lightNode = node["DirectionalLightComponent"];
			{
				auto& light = entity.GetComponent<DirectionalLightComponent>();
				lightNode["Color"] = light.color;
			}
		}

		if (entity.HasComponent<CameraComponent>())
		{
			auto& camNode = node["CameraComponent"];
			{
				auto& cam = entity.GetComponent<CameraComponent>();
				camNode["CameraPos"] = cam.pCamera->GetPosition();
				if (cam.type == ECameraType::eFirstPerson)
				{
					auto* pFPSCam = static_cast<FirstPersonCamera*>(cam.pCamera.get());
					camNode["UpVector"] = pFPSCam->GetUpVector();
					camNode["RightVector"] = pFPSCam->GetRightVector();
					camNode["FrontVector"] = pFPSCam->GetFrontVector();
				}
			}
		}
	}

	Scene::Scene()
	{
		mEntityManager.GetComponentHooker<MeshComponent>().HookOnAdded(_OnMeshComponentAdded);
		mEntityManager.GetComponentHooker<MeshComponent>().HookOnRemoved(_OnMeshComponentRemoved);
	}

	void Scene::OnSceneLoad()
	{
		SG_PROFILE_FUNCTION();

		mSkyboxEntity = mEntityManager.CreateEntity();
		mSkyboxEntity.AddComponent<TagComponent>("__skybox");
		auto& mesh = mSkyboxEntity.AddComponent<MeshComponent>();
		LoadMesh(EGennerateMeshType::eSkybox, mesh);

		//const auto camPos = Vector3f(0.0f, 3.0f, 7.0f);

		//mpCameraEntity = CreateEntity("main_camera", camPos, Vector3f(1.0f), Vector3f(0.0f));
		//auto& cam = mpCameraEntity->AddComponent<CameraComponent>();
		//cam.pCamera = MakeRef<FirstPersonCamera>(camPos);
		//cam.pCamera->SetPerspective(60.0f, OperatingSystem::GetMainWindow()->GetAspectRatio());

		//auto* pEntity = CreateEntity("directional_light_0", Vector3f{ 8.0f, 12.0f, 0.0f }, Vector3f(1.0f), Vector3f(40.0f, -40.0f, 0.0f));
		//pEntity->AddTag<LightTag>();
		//pEntity->AddComponent<DirectionalLightComponent>();

		//pEntity = CreateEntity("point_light_0", { 1.25f, 0.75f, -0.3f }, Vector3f(1.0f), Vector3f(0.0f));
		//pEntity->AddTag<LightTag>();
		//auto& pointLight = pEntity->AddComponent<PointLightComponent>();
		//pointLight.radius = 3.0f;
		//pointLight.color = { 0.0f, 1.0f, 0.705f };

		//MaterialScene();
		//MaterialTexturedScene();
		//DefaultScene();

		Refresh();
	}

	void Scene::OnSceneUnLoad()
	{
		SG_PROFILE_FUNCTION();

		// clear all the object id in the mesh.
		gObjectIdAllocator.Reset();
	}

	void Scene::OnUpdate(float deltaTime)
	{
		SG_PROFILE_FUNCTION();

		mpCameraEntity->GetComponent<CameraComponent>().pCamera->OnUpdate(deltaTime);

		//static float totalTime = 0.0f;
		//static float speed = 2.5f;

		//// Do animation
		//auto* pModel = GetEntityByName("model");
		//auto [tag, trans] = pModel->GetComponent<TagComponent, TransformComponent>();
		//tag.bDirty = true;
		//trans.position = { Sin(totalTime) * 0.5f, 0.0f, 0.0f };

		//totalTime += deltaTime * speed;

		mEntityManager.ReFresh();
	}

	Scene::Entity* Scene::CreateEntity(const string& name)
	{
		SG_PROFILE_FUNCTION();

		if (mEntities.find(name) != mEntities.end())
		{
			SG_LOG_WARN("Already have an entity named: %s", name.c_str());
			return nullptr;
		}
		auto entity = mEntityManager.CreateEntity();
		// an entity must have a TagComponent and a TransformComponent
		entity.AddComponent<TagComponent>(name);
		entity.AddComponent<TransformComponent>();

		mEntities[name] = entity;
		return &mEntities[name];
	}

	Scene::Entity* Scene::CreateEntity(const string& name, const Vector3f& pos, const Vector3f& scale, const Vector3f& rot)
	{
		SG_PROFILE_FUNCTION();

		if (mEntities.find(name) != mEntities.end())
		{
			SG_LOG_WARN("Already have an entity named: %s", name.c_str());
			return nullptr;
		}
		auto entity = mEntityManager.CreateEntity();
		// an entity must have a TagComponent and a TransformComponent
		entity.AddComponent<TagComponent>(name);
		entity.AddComponent<TransformComponent>(pos, scale, rot);

		mEntities[name] = entity;
		return &mEntities[name];
	}

	void Scene::DestroyEntity(Entity& entity)
	{
		SG_PROFILE_FUNCTION();

		auto& tag = entity.GetComponent<TagComponent>();
		mEntities.erase(tag.name);
		mEntityManager.DestroyEntity(entity);
	}

	void Scene::DestroyEntityByName(const string& name)
	{
		SG_PROFILE_FUNCTION();

		auto* pEntity = GetEntityByName(name);
		mEntityManager.DestroyEntity(*pEntity);
		mEntities.erase(name);
	}

	Scene::Entity* Scene::GetEntityByName(const string& name)
	{
		SG_PROFILE_FUNCTION();

		auto node = mEntities.find(name);
		if (node == mEntities.end())
		{
			SG_LOG_WARN("No entity named: %s", name.c_str());
			return nullptr;
		}
		return &node->second;
	}

	void Scene::DefaultScene()
	{
		SG_PROFILE_FUNCTION();

		auto* pEntity = CreateEntity("model");
		pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.2f, 0.8f);
		auto& mesh = pEntity->AddComponent<MeshComponent>();
		LoadMesh("model", EMeshType::eOBJ, mesh);

		pEntity = CreateEntity("model_1", { 0.0f, 0.0f, -1.5f }, { 0.6f, 0.6f, 0.6f }, Vector3f(0.0f));
		pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.8f, 0.35f);
		auto& mesh1 = pEntity->AddComponent<MeshComponent>();
		CopyMesh(GetEntityByName("model")->GetComponent<MeshComponent>(), mesh1);

		pEntity = CreateEntity("grid", Vector3f(0.0f), { 8.0f, 1.0f, 8.0f }, Vector3f(0.0f));
		auto& mat = pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.05f, 0.76f);
		mat.normalTex = TextureAssetArchive::GetInstance()->NewTextureAsset("cerberus_normal", "cerberus/normal.ktx", true);
		auto& mesh2 = pEntity->AddComponent<MeshComponent>();
		LoadMesh(EGennerateMeshType::eGrid, mesh2);
	}

	void Scene::MaterialScene()
	{
		SG_PROFILE_FUNCTION();

		const float zPos = -4.0f;
		const float INTERVAL = 1.5f;
		for (UInt32 i = 0; i < 10; ++i)
		{
			for (UInt32 j = 0; j < 10; ++j)
			{
				string name = "sphere" + eastl::to_string(i) + "_" + eastl::to_string(j);
				if (i == 0 && j == 0)
				{
					auto* pEntity = CreateEntity(name, { -7.0f + i * INTERVAL, -7.0f + j * INTERVAL, zPos }, { 0.7f, 0.7f, 0.7f }, Vector3f(0.0f));
					auto& mesh = pEntity->AddComponent<MeshComponent>();
					LoadMesh("sphere", EMeshType::eOBJ, mesh);

					pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), (i + 1) * 0.1f, (10 - j) * 0.1f);
				}
				else
				{
					auto* pEntity = CreateEntity(name, { -7.0f + i * INTERVAL, -7.0f + j * INTERVAL, zPos }, { 0.7f, 0.7f, 0.7f }, Vector3f(0.0f));
					auto& mesh = pEntity->AddComponent<MeshComponent>();

					auto* pCopyEntity = GetEntityByName("sphere0_0");
					auto& srcMesh = pCopyEntity->GetComponent<MeshComponent>();
					CopyMesh(srcMesh, mesh);

					pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), (i + 1) * 0.1f, (10 - j) * 0.1f);
				}
			}
		}
	}

	void Scene::MaterialTexturedScene()
	{
		auto* pEntity = CreateEntity("cerberus");
		auto& trans = pEntity->GetComponent<TransformComponent>();
		trans.position.x = 2.7f;
		trans.position.y = 3.0f;
		trans.rotation.y = 90.0f;
		trans.scale = { 3.0f, 3.0f, 3.0f };
		auto& mat = pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.05f, 0.9f);
		mat.albedoTex = TextureAssetArchive::GetInstance()->NewTextureAsset("cerberus_albedo", "cerberus/albedo.ktx", true);
		mat.normalTex = TextureAssetArchive::GetInstance()->NewTextureAsset("cerberus_normal", "cerberus/normal.ktx", true);
		mat.metallicTex = TextureAssetArchive::GetInstance()->NewTextureAsset("cerberus_metallic", "cerberus/metallic.ktx", true);
		mat.roughnessTex = TextureAssetArchive::GetInstance()->NewTextureAsset("cerberus_roughness", "cerberus/roughness.ktx", true);
		mat.AOTex = TextureAssetArchive::GetInstance()->NewTextureAsset("cerberus_ao", "cerberus/ao.ktx", true);
		auto& mesh = pEntity->AddComponent<MeshComponent>();
		LoadMesh("cerberus", EMeshType::eOBJ, mesh);

		pEntity = CreateEntity("cerberus_1");
		auto& trans1 = pEntity->GetComponent<TransformComponent>();
		trans1.position.x = 2.6f;
		trans1.position.y = 1.85f;
		trans1.position.z = -2.3f;
		trans1.rotation.y = 90.0f;
		trans1.scale = { 3.0f, 3.0f, 3.0f };
		auto& mat1 = pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.05f, 0.9f);
		mat1.AOTex = TextureAssetArchive::GetInstance()->NewTextureAsset("cerberus_ao", "cerberus/ao.ktx", true);
		auto& mesh1 = pEntity->AddComponent<MeshComponent>();
		CopyMesh(GetEntityByName("cerberus")->GetComponent<MeshComponent>(), mesh1);
	}

	void Scene::Serialize(json& node)
	{
		SG_PROFILE_FUNCTION();

		node["Scene"] = "My Default Scene";
		node["Entities"] = {};

		TraverseEntity([&](auto& entity)
			{
				_SerializeEntity(entity, node["Entities"].emplace_back());
			});
	}

	void Scene::Deserialize(json& node)
	{
		SG_PROFILE_FUNCTION();

		string sceneName = node["Scene"].get<std::string>().c_str();
		auto& entities = node["Entities"];
		if (entities.is_array() && !entities.empty())
		{
			for (auto& entity : entities)
			{
				auto& tagComp = entity["TagComponent"];
				string name = tagComp["Name"].get<std::string>().c_str();

				auto& transComp = entity["TransformComponent"];
				auto* pEntity = CreateEntity(name, transComp["Position"].get<Vector3f>(), transComp["Scale"].get<Vector3f>(), transComp["Rotation"].get<Vector3f>());

				if (auto node = entity.find("MeshComponent"); node != entity.end())
				{
					auto& meshComp = *node;

					string filename = meshComp["Filename"].get<std::string>().c_str();
					bool bIsProceduralMesh = meshComp["IsProceduralMesh"].get<bool>();
					auto& mesh = pEntity->AddComponent<MeshComponent>();

					if (MeshDataArchive::GetInstance()->HaveMeshData(filename))
					{
						if (bIsProceduralMesh)
							mesh.meshType = EMeshType::eUnknown;
						else
							mesh.meshType = EMeshType::eOBJ;
						mesh.meshId = MeshDataArchive::GetInstance()->GetMeshID(filename);
					}
					else
					{
						if (bIsProceduralMesh)
						{
							if (filename == "_generated_grid")
								LoadMesh(EGennerateMeshType::eGrid, mesh);
							else if (filename == "_generated_skybox")
								LoadMesh(EGennerateMeshType::eSkybox, mesh);
						}
						else
						{
							LoadMesh(filename.c_str(), EMeshType::eOBJ, mesh);
						}
					}
				}

				if (auto node = entity.find("MaterialComponent"); node != entity.end())
				{
					auto& matComp = *node;

					auto& mat = pEntity->AddComponent<MaterialComponent>();

					matComp["Albedo"].get_to(mat.albedo);
					matComp["Metallic"].get_to(mat.metallic);
					matComp["Roughness"].get_to(mat.roughness);

					if (auto n = matComp.find("AlbedoTextureAsset"); n != matComp.end())
					{
						auto& node = *n;
						string filename = node["Filename"].get<std::string>().c_str();

						mat.albedoTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.albedoTex->Deserialize(node);
					}
					else
						mat.albedoTex = nullptr;

					if (auto n = matComp.find("MetallicTextureAsset"); n != matComp.end())
					{
						auto& node = *n;
						string filename = node["Filename"].get<std::string>().c_str();

						mat.metallicTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.metallicTex->Deserialize(node);
					}
					else
						mat.metallicTex = nullptr;

					if (auto n = matComp.find("RoughnessTextureAsset"); n != matComp.end())
					{
						auto& node = *n;
						string filename = node["Filename"].get<std::string>().c_str();

						mat.roughnessTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.roughnessTex->Deserialize(node);
					}
					else
						mat.roughnessTex = nullptr;

					if (auto n = matComp.find("NormalTextureAsset"); n != matComp.end())
					{
						auto& node = *n;
						string filename = node["Filename"].get<std::string>().c_str();

						mat.normalTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.normalTex->Deserialize(node);
					}
					else
						mat.normalTex = nullptr;

					if (auto n = matComp.find("AOTextureAsset"); n != matComp.end())
					{
						auto& node = *n;
						string filename = node["Filename"].get<std::string>().c_str();

						mat.AOTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.AOTex->Deserialize(node);
					}
					else
						mat.AOTex = nullptr;
				}

				if (auto node = entity.find("PointLightComponent"); node != entity.end())
				{
					auto& pointLightComp = *node;

					auto& light = pEntity->AddComponent<PointLightComponent>();
					pointLightComp["Color"].get_to<Vector3f>(light.color);
					pointLightComp["Radius"].get_to(light.radius);
				}

				if (auto node = entity.find("DirectionalLightComponent"); node != entity.end())
				{
					auto& directionalLightComp = *node;

					auto& light = pEntity->AddComponent<DirectionalLightComponent>();
					directionalLightComp["Color"].get_to(light.color);
				}

				if (auto node = entity.find("CameraComponent"); node != entity.end())
				{
					auto& cameraComp = *node;
					auto& cam = pEntity->AddComponent<CameraComponent>();

					cam.type = ECameraType::eFirstPerson;
					auto FPSCam = MakeRef<FirstPersonCamera>(cameraComp["CameraPos"].get<Vector3f>());
					FPSCam->SetPerspective(60.0f, OperatingSystem::GetMainWindow()->GetAspectRatio()); // TODO: Wrong aspect ratio
					FPSCam->SetUpVector(cameraComp["UpVector"].get<Vector3f>());
					FPSCam->SetRightVector(cameraComp["RightVector"].get<Vector3f>());
					FPSCam->SetFrontVector(cameraComp["FrontVector"].get<Vector3f>());
					cam.pCamera = FPSCam;
					mpCameraEntity = pEntity;
				}
			}
		}

		Refresh();
	}

	void Scene::Refresh()
	{
		mEntityManager.ReFresh();

		mMeshEntityCount = 0;
		for (auto node : mEntities)
		{
			auto& entity = node.second;
			if (entity.HasComponent<MeshComponent>())
				++mMeshEntityCount;
		}
	}

}