#include "StdAfx.h"
#include "Scene/Scene.h"

#include "Platform/OS.h"
#include "Scene/Camera/FirstPersonCamera.h"
#include "Scene/Mesh/MeshDataArchive.h"

namespace SG
{

	void Scene::OnSceneLoad()
	{
		// camera initialize
		auto* window = OperatingSystem::GetMainWindow();
		const float ASPECT = window->GetAspectRatio();

		mpMainCamera = MakeRef<FirstPersonCamera>(Vector3f(0.0f, 0.0f, 15.0f));
		mpMainCamera->SetPerspective(45.0f, ASPECT, 0.01f, 256.0f);

		mPointLights.emplace_back(Vector3f{ 1.25f, 0.75f, -0.3f }, 3.0f, Vector3f{ 0.0f, 1.0f, 0.705f });

		const float INTERVAL = 1.5f;
		for (UInt32 i = 0; i < 10; ++i)
		{
			for (UInt32 j = 0; j < 10; ++j)
			{
				string name = "sphere" + eastl::to_string(i) + "_" + eastl::to_string(j);
				if (i == 0 && j == 0)
				{
					auto& mesh = mMeshes.emplace(name.c_str(), Mesh(name.c_str(), "sphere", EMeshType::eOBJ)).first->second;
					mesh.SetPosition({ -7.0f + i * INTERVAL, -7.0f + j * INTERVAL, 0.0f });
					mesh.SetScale({ 0.7f, 0.7f, 0.7f });
					mesh.SetMetallic((i + 1) * 0.1f);
					mesh.SetRoughness((10 - j) * 0.1f);
				}
				else
				{
					auto& mesh = mMeshes.emplace(name.c_str(), Mesh(name.c_str())).first->second;
					mesh.Copy(*GetMesh("sphere0_0"));
					mesh.SetPosition({ -7.0f + i * INTERVAL, -7.0f + j * INTERVAL, 0.0f });
					mesh.SetScale({ 0.7f, 0.7f, 0.7f });
					mesh.SetMetallic((i + 1) * 0.1f);
					mesh.SetRoughness((10 - j) * 0.1f);
				}
			}
		}

		//auto& model = mMeshes.emplace("model", Mesh("model", EMeshType::eOBJ)).first->second;
		//auto& model_1 = mMeshes.emplace("model_1", Mesh("model_1")).first->second;
		//model_1.Copy(model);
		//model_1.SetPosition({ 0.0f, 0.0f, -1.5f });
		//model_1.SetScale({ 0.6f, 0.6f, 0.6f });
		//mMeshes.emplace("grid", Mesh("grid", EGennerateMeshType::eGrid)).first->second.SetScale({ 8.0f, 1.0f, 8.0f });
	}

	void Scene::OnSceneUnLoad()
	{
		for (UInt32 i = 0; i < 10; ++i)
		{
			for (UInt32 j = 0; j < 10; ++j)
			{
				MeshDataArchive::GetInstance()->DecreaseRef(GetMesh("sphere0_0")->GetMeshID());
			}
		}
		// temporary
		//MeshDataArchive::GetInstance()->DecreaseRef(GetMesh("model")->GetMeshID());
		//MeshDataArchive::GetInstance()->DecreaseRef(GetMesh("model")->GetMeshID());
	}

	void Scene::OnUpdate(float deltaTime)
	{
		mpMainCamera->OnUpdate(deltaTime);

		//static float totalTime = 0.0f;
		//static float speed = 2.5f;

		// Do animation
		//Mesh* pModel = GetMesh("model");
		//pModel->SetPosition({ Sin(totalTime) * 0.5f, 0.0f, 0.0f });

		//totalTime += deltaTime * speed;
	}

	Mesh* Scene::GetMesh(const char* name)
	{
		auto pNode = mMeshes.find(name);
		if (pNode != mMeshes.end())
			return &pNode->second;
		SG_LOG_WARN("Try to get non-exist mesh: %s", name);
		return nullptr;
	}

}