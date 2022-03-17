#include "StdAfx.h"
#include "Scene/Scene.h"

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

		mpMainCamera = MakeRef<FirstPersonCamera>(Vector3f(0.0f, 2.0f, -4.0f));
		mpMainCamera->SetPerspective(45.0f, ASPECT, 0.01f, 256.0f);

		auto& model = mMeshes.emplace_back("model", EMeshType::eOBJ);
		model.SetRotation({ 0.0, 180.0f, 0.0f });
		auto& grid = mMeshes.emplace_back("grid", EGennerateMeshType::eGrid);
		grid.SetScale({ 8.0f, 1.0f, 8.0f });

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
		// TODO: this way of setting a value of a given mesh is too silly
		TraverseMesh([](Mesh& mesh)
			{
				if (mesh.GetName() == "model")
				{
					mesh.SetPosition({ 0.5f * Sin(totalTime), 0.0f, 0.0f });
					return;
				}
			});

		totalTime += deltaTime * speed;
	}

}