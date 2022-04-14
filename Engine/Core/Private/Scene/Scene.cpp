#include "StdAfx.h"
#include "Scene/Scene.h"

#include "Platform/OS.h"
#include "Scene/Camera/FirstPersonCamera.h"
#include "Scene/Mesh/MeshDataArchive.h"

namespace SG
{

	UInt32 Scene::gCurrObjectID = 0;

	void Scene::OnSceneLoad()
	{
		// camera initialize
		auto* window = OperatingSystem::GetMainWindow();
		const float ASPECT = window->GetAspectRatio();

		mSkyboxEntity = mEntityManager.CreateEntity();
		mSkyboxEntity.AddComponent<TagComponent>("skybox");
		auto& mesh = mSkyboxEntity.AddComponent<MeshComponent>();
		LoadMesh(EGennerateMeshType::eSkybox, mesh);
		mesh.objectId = NewObjectID();

		mpMainCamera = MakeRef<FirstPersonCamera>(Vector3f(0.0f, 0.5f, 7.0f));
		mpMainCamera->SetPerspective(60.0f, ASPECT, 0.01f, 256.0f);

		auto* pEntity = CreateEntity("directional_light_0", Vector3f{ .0f, 12.0f, 0.0f }, Vector3f(1.0f), Vector3f(40.0f, -40.0f, 0.0f));
		pEntity->AddTag<LightTag>();
		pEntity->AddComponent<DirectionalLightComponent>();

		pEntity = CreateEntity("point_light_0", { 1.25f, 0.75f, -0.3f }, Vector3f(1.0f), Vector3f(0.0f));
		pEntity->AddTag<LightTag>();
		auto& pointLight = pEntity->AddComponent<PointLightComponent>();
		pointLight.radius = 3.0f;
		pointLight.color = { 0.0f, 1.0f, 0.705f };
		pEntity->GetComponent<TagComponent>().bDirty = true;

		DefaultScene();
		MaterialTestScene();

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
	}

	void Scene::OnUpdate(float deltaTime)
	{
		mpMainCamera->OnUpdate(deltaTime);

		//static float totalTime = 0.0f;
		//static float speed = 2.5f;

		//// Do animation
		//auto pModel = GetMesh("model");
		//pModel->SetPosition({ Sin(totalTime) * 0.5f, 0.0f, 0.0f });

		//totalTime += deltaTime * speed;
		mEntityManager.ReFresh();
	}

	Scene::Entity* Scene::CreateEntity(const string& name)
	{
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
		auto& tag = entity.GetComponent<TagComponent>();
		mEntities.erase(tag.name);
		mEntityManager.DestroyEntity(entity);
	}

	void Scene::DestroyEntityByName(const string& name)
	{
		auto* pEntity = GetEntityByName(name);
		mEntityManager.DestroyEntity(*pEntity);
		mEntities.erase(name);
	}

	Scene::Entity* Scene::GetEntityByName(const string& name)
	{
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
		auto* pEntity = CreateEntity("model");
		pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.2f, 0.8f);
		auto& mesh = pEntity->AddComponent<MeshComponent>();
		mesh.objectId = NewObjectID();
		LoadMesh("model", EMeshType::eOBJ, mesh);

		pEntity = CreateEntity("model_1", { 0.0f, 0.0f, -1.5f }, { 0.6f, 0.6f, 0.6f }, Vector3f(0.0f));
		pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.8f, 0.35f);
		auto& mesh1 = pEntity->AddComponent<MeshComponent>();
		mesh1.objectId = NewObjectID();
		CopyMesh(GetEntityByName("model")->GetComponent<MeshComponent>(), mesh1);

		pEntity = CreateEntity("grid", Vector3f(0.0f), { 8.0f, 1.0f, 8.0f }, Vector3f(0.0f));
		pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), 0.05f, 0.76f);
		auto& mesh2 = pEntity->AddComponent<MeshComponent>();
		mesh2.objectId = NewObjectID();
		LoadMesh(EGennerateMeshType::eGrid, mesh2);
	}

	void Scene::MaterialTestScene()
	{
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
					mesh.objectId = NewObjectID();
					LoadMesh("sphere", EMeshType::eOBJ, mesh);

					pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), (i + 1) * 0.1f, (10 - j) * 0.1f);
				}
				else
				{
					auto* pEntity = CreateEntity(name, { -7.0f + i * INTERVAL, -7.0f + j * INTERVAL, zPos }, { 0.7f, 0.7f, 0.7f }, Vector3f(0.0f));
					auto& mesh = pEntity->AddComponent<MeshComponent>();
					mesh.objectId = NewObjectID();

					auto* pCopyEntity = GetEntityByName("sphere0_0");
					auto& srcMesh = pCopyEntity->GetComponent<MeshComponent>();
					CopyMesh(srcMesh, mesh);

					pEntity->AddComponent<MaterialComponent>(Vector3f(1.0f), (i + 1) * 0.1f, (10 - j) * 0.1f);
				}
			}
		}
	}

}