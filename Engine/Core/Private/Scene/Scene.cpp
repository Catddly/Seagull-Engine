#include "StdAfx.h"
#include "Scene/Scene.h"

#include "Memory/Memory.h"
#include "Platform/OS.h"
#include "Scene/Camera/PointOrientedCamera.h"
#include "Scene/Camera/FirstPersonCamera.h"
#include "Render/MeshGenerate/MeshGenerator.h"

namespace SG
{

	void Scene::OnSceneLoad()
	{
		// camera initialize
		auto* window = OperatingSystem::GetMainWindow();
		const float ASPECT = window->GetAspectRatio();

		mpMainCamera = MakeRef<FirstPersonCamera>(Vector3f(0.0f, 0.5f, 4.0f));
		mpMainCamera->SetPerspective(45.0f, ASPECT, 0.01f, 256.0f);

		mMeshes.emplace("model", Mesh("model", EMeshType::eOBJ));
		mMeshes.emplace("skybox", Mesh("skybox", EGennerateMeshType::eSkybox));
		mMeshes.emplace("grid", Mesh("grid", EGennerateMeshType::eGrid)).first->second.SetScale({ 8.0f, 1.0f, 8.0f });

		mPointLights.emplace_back(Vector3f{ -0.85f, 3.0f, -1.5f }, 1.5f, Vector3f{ 0.159f, 0.3f, 0.755f });
	}

	void Scene::OnSceneUnLoad()
	{

	}

	void Scene::OnUpdate(float deltaTime)
	{
		mpMainCamera->OnUpdate(deltaTime);

		static float totalTime = 0.0f;
		static float speed = 2.5f;

		// Do animation
		Mesh* pModel = GetMesh("model");
		pModel->SetPosition({ Sin(totalTime) * 0.5f, 0.0f, 0.0f });

		totalTime += deltaTime * speed;
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