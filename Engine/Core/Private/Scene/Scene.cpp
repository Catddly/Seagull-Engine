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

	static void _SerializeEntity(Scene::Entity& entity, YAML::Emitter& outStream)
	{
		outStream << YAML::BeginMap;
		outStream << YAML::Key << "Entity" << YAML::Value << "9468552411547745"; // entity id goes here

		outStream << YAML::Key << "TagComponent";
		outStream << YAML::BeginMap;
		{
			auto& tag = entity.GetComponent<TagComponent>();
			outStream << YAML::Key << "Name" << YAML::Value << tag.name.c_str();
		}
		outStream << YAML::EndMap;

		outStream << YAML::Key << "TransformComponent";
		outStream << YAML::BeginMap;
		{
			auto& trans = entity.GetComponent<TransformComponent>();
			outStream << YAML::Key << "Position" << YAML::Value << trans.position;
			outStream << YAML::Key << "Rotation" << YAML::Value << trans.rotation;
			outStream << YAML::Key << "Scale" << YAML::Value << trans.scale;
		}
		outStream << YAML::EndMap;

		if (entity.HasComponent<MeshComponent>())
		{
			outStream << YAML::Key << "MeshComponent";
			outStream << YAML::BeginMap;
			{
				auto& mesh = entity.GetComponent<MeshComponent>();
				auto* pMeshData = MeshDataArchive::GetInstance()->GetData(mesh.meshId);
				outStream << YAML::Key << "Filename" << YAML::Value << pMeshData->filename.c_str();
				outStream << YAML::Key << "IsProceduralMesh" << YAML::Value << pMeshData->bIsProceduralMesh;
			}
			outStream << YAML::EndMap;
		}

		if (entity.HasComponent<MaterialComponent>())
		{
			outStream << YAML::Key << "MaterialComponent";
			outStream << YAML::BeginMap;
			{
				auto& mat = entity.GetComponent<MaterialComponent>();
				outStream << YAML::Key << "Albedo" << YAML::Value << mat.albedo;
				if (mat.albedoTex)
				{
					outStream << YAML::Key << "AlbedoTextureAsset";
					outStream << YAML::BeginMap;
					mat.albedoTex->Serialize(outStream);
					outStream << YAML::EndMap;
				}
				outStream << YAML::Key << "Metallic" << YAML::Value << mat.metallic;
				if (mat.metallicTex)
				{
					outStream << YAML::Key << "MetallicTextureAsset";
					outStream << YAML::BeginMap;
					mat.metallicTex->Serialize(outStream);
					outStream << YAML::EndMap;
				}
				outStream << YAML::Key << "Roughness" << YAML::Value << mat.roughness;
				if (mat.roughnessTex)
				{
					outStream << YAML::Key << "RoughnessTextureAsset";
					outStream << YAML::BeginMap;
					mat.roughnessTex->Serialize(outStream);
					outStream << YAML::EndMap;
				}
				if (mat.normalTex)
				{
					outStream << YAML::Key << "NormalTextureAsset";
					outStream << YAML::BeginMap;
					mat.normalTex->Serialize(outStream);
					outStream << YAML::EndMap;
				}
				if (mat.AOTex)
				{
					outStream << YAML::Key << "AOTextureAsset";
					outStream << YAML::BeginMap;
					mat.AOTex->Serialize(outStream);
					outStream << YAML::EndMap;
				}

			}
			outStream << YAML::EndMap;
		}

		if (entity.HasComponent<PointLightComponent>())
		{
			outStream << YAML::Key << "PointLightComponent";
			outStream << YAML::BeginMap;
			{
				auto& light = entity.GetComponent<PointLightComponent>();
				outStream << YAML::Key << "Color" << YAML::Value << light.color;
				outStream << YAML::Key << "Radius" << YAML::Value << light.radius;
			}
			outStream << YAML::EndMap;
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			outStream << YAML::Key << "DirectionalLightComponent";
			outStream << YAML::BeginMap;
			{
				auto& mat = entity.GetComponent<DirectionalLightComponent>();
				outStream << YAML::Key << "Color" << YAML::Value << mat.color;
			}
			outStream << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			outStream << YAML::Key << "CameraComponent";
			outStream << YAML::BeginMap;
			{
				auto& cam = entity.GetComponent<CameraComponent>();
				outStream << YAML::Key << "CameraPos" << YAML::Value << cam.pCamera->GetPosition();
				if (cam.type == ECameraType::eFirstPerson)
				{
					auto* pFPSCam = static_cast<FirstPersonCamera*>(cam.pCamera.get());
					outStream << YAML::Key << "UpVector" << YAML::Value << pFPSCam->GetUpVector();
					outStream << YAML::Key << "RightVector" << YAML::Value << pFPSCam->GetRightVector();
					outStream << YAML::Key << "FrontVector" << YAML::Value << pFPSCam->GetFrontVector();
				}
			}
			outStream << YAML::EndMap;
		}

		outStream << YAML::EndMap;
	}

	Scene::Scene()
	{
		mEntityManager.GetComponentHooker<MeshComponent>().HookOnAdded(_OnMeshComponentAdded);
		mEntityManager.GetComponentHooker<MeshComponent>().HookOnRemoved(_OnMeshComponentRemoved);
	}

	void Scene::OnSceneLoad()
	{
		SG_PROFILE_FUNCTION();

		//mpMainCamera = MakeRef<FirstPersonCamera>(Vector3f(0.0f, 3.0f, 7.0f));
		//mpMainCamera->SetPerspective(60.0f, OperatingSystem::GetMainWindow()->GetAspectRatio());

		//const auto camPos = Vector3f(0.0f, 3.0f, 7.0f);

		//mpCameraEntity = CreateEntity("main_camera", camPos, Vector3f(1.0f), Vector3f(0.0f));
		//auto& cam = mpCameraEntity->AddComponent<CameraComponent>();
		//cam.pCamera = MakeRef<FirstPersonCamera>(camPos);
		//cam.pCamera->SetPerspective(60.0f, OperatingSystem::GetMainWindow()->GetAspectRatio());

		//mCameraEntity = mEntityManager.CreateEntity();
		//mCameraEntity.AddComponent<TagComponent>("main_camera");
		//mCameraEntity.AddComponent<TransformComponent>(camPos, Vector3f(1.0f), Vector3f(0.0f));

		mSkyboxEntity = mEntityManager.CreateEntity();
		mSkyboxEntity.AddComponent<TagComponent>("skybox");
		auto& mesh = mSkyboxEntity.AddComponent<MeshComponent>();
		LoadMesh(EGennerateMeshType::eSkybox, mesh);

		//auto* pEntity = CreateEntity("directional_light_0", Vector3f{ 8.0f, 12.0f, 0.0f }, Vector3f(1.0f), Vector3f(40.0f, -40.0f, 0.0f));
		//pEntity->AddTag<LightTag>();
		//pEntity->AddComponent<DirectionalLightComponent>();

		//pEntity = CreateEntity("point_light_0", { 1.25f, 0.75f, -0.3f }, Vector3f(1.0f), Vector3f(0.0f));
		//pEntity->AddTag<LightTag>();
		//auto& pointLight = pEntity->AddComponent<PointLightComponent>();
		//pointLight.radius = 3.0f;
		//pointLight.color = { 0.0f, 1.0f, 0.705f };
		//pEntity->GetComponent<TagComponent>().bDirty = true;

		//DefaultScene();
		//MaterialScene();
		//MaterialTexturedScene();

		mEntityManager.ReFresh();

		for (auto node : mEntities)
		{
			auto& entity = node.second;
			if (entity.HasComponent<MeshComponent>())
				++mMeshEntityCount;
		}
	}

	void Scene::OnSceneUnLoad()
	{
		SG_PROFILE_FUNCTION();
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
		pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.05f, 0.76f);
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

	void Scene::Serialize(YAML::Emitter& outStream)
	{
		outStream << YAML::BeginMap;
		outStream << YAML::Key << "Scene" << YAML::Value << "My Test Scene";
		outStream << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		{
			TraverseEntity([&](auto& entity)
				{
					_SerializeEntity(entity, outStream);
				});
		}
		outStream << YAML::EndSeq;
		outStream << YAML::EndMap;
	}

	void Scene::Deserialize(YAML::Node& node)
	{
		string sceneName = node["Scene"].as<std::string>().c_str();
		auto entities = node["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				auto tagComp = entity["TagComponent"];
				string name = tagComp["Name"].as<std::string>().c_str();

				auto transComp = entity["TransformComponent"];
				auto* pEntity = CreateEntity(name, transComp["Position"].as<Vector3f>(), transComp["Scale"].as<Vector3f>(), transComp["Rotation"].as<Vector3f>());

				auto meshComp = entity["MeshComponent"];
				if (meshComp)
				{
					string filename = meshComp["Filename"].as<std::string>().c_str();
					bool bIsProceduralMesh = meshComp["IsProceduralMesh"].as<bool>();
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

				auto matComp = entity["MaterialComponent"];
				if (matComp)
				{
					auto& mat = pEntity->AddComponent<MaterialComponent>();

					mat.albedo = matComp["Albedo"].as<Vector3f>();
					if (matComp["AlbedoTextureAsset"])
					{
						auto node = matComp["AlbedoTextureAsset"];
						string filename = node["Filename"].as<std::string>().c_str();

						mat.albedoTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.albedoTex->Deserialize(node);
					}
					else
						mat.albedoTex = nullptr;

					mat.metallic = matComp["Metallic"].as<float>();
					if (matComp["MetallicTextureAsset"])
					{
						auto node = matComp["MetallicTextureAsset"];
						string filename = node["Filename"].as<std::string>().c_str();

						mat.metallicTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.metallicTex->Deserialize(node);
					}
					else
						mat.metallicTex = nullptr;

					mat.roughness = matComp["Roughness"].as<float>();
					if (matComp["RoughnessTextureAsset"])
					{
						auto node = matComp["RoughnessTextureAsset"];
						string filename = node["Filename"].as<std::string>().c_str();

						mat.roughnessTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.roughnessTex->Deserialize(node);
					}
					else
						mat.roughnessTex = nullptr;

					if (matComp["NormalTextureAsset"])
					{
						auto node = matComp["NormalTextureAsset"];
						string filename = node["Filename"].as<std::string>().c_str();

						mat.normalTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.normalTex->Deserialize(node);
					}
					else
						mat.normalTex = nullptr;

					if (matComp["AOTextureAsset"])
					{
						auto node = matComp["AOTextureAsset"];
						string filename = node["Filename"].as<std::string>().c_str();

						mat.AOTex = TextureAssetArchive::GetInstance()->NewTextureAsset("", filename);
						mat.AOTex->Deserialize(node);
					}
					else
						mat.AOTex = nullptr;
				}

				auto pointLightComp = entity["PointLightComponent"];
				if (pointLightComp)
				{
					auto& light = pEntity->AddComponent<PointLightComponent>();
					light.color = pointLightComp["Color"].as<Vector3f>();
					light.radius = pointLightComp["Radius"].as<float>();
				}

				auto directionalLightComp = entity["DirectionalLightComponent"];
				if (directionalLightComp)
				{
					auto& light = pEntity->AddComponent<DirectionalLightComponent>();
					light.color = directionalLightComp["Color"].as<Vector3f>();
				}

				auto cameraComp = entity["CameraComponent"];
				if (cameraComp)
				{
					auto& cam = pEntity->AddComponent<CameraComponent>();
					cam.type = ECameraType::eFirstPerson;
					auto FPSCam = MakeRef<FirstPersonCamera>(cameraComp["CameraPos"].as<Vector3f>());
					FPSCam->SetPerspective(60.0f, OperatingSystem::GetMainWindow()->GetAspectRatio());
					FPSCam->SetUpVector(cameraComp["UpVector"].as<Vector3f>());
					FPSCam->SetRightVector(cameraComp["RightVector"].as<Vector3f>());
					FPSCam->SetFrontVector(cameraComp["FrontVector"].as<Vector3f>());
					cam.pCamera = FPSCam;
					mpCameraEntity = pEntity;
				}
			}
		}

		mEntityManager.ReFresh();

		for (auto node : mEntities)
		{
			auto& entity = node.second;
			if (entity.HasComponent<MeshComponent>())
				++mMeshEntityCount;
		}
	}

}