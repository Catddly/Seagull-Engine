#include "StdAfx.h"
#include "Scene/Scene.h"

#include "Platform/OS.h"
#include "Scene/Camera/PointOrientedCamera.h"

namespace SG
{

	void Scene::OnSceneLoad()
	{
		// camera initialize
		auto* window = OperatingSystem::GetMainWindow();
		const float ASPECT = window->GetAspectRatio();

		mpMainCamera = MakeRef<PointOrientedCamera>(Vector3f(0.0f, 0.0f, -4.0f));
		mpMainCamera->SetPerspective(45.0f, ASPECT);

		mMeshes.emplace_back("model", EMeshType::eOBJ);

		mPointLights.emplace_back(Vector3f{ 1.2f, 2.0f, 1.0f }, 10.0f,
			Vector3f{ 1.0f, 1.0f, 1.0f });
	}

	void Scene::OnSceneUnLoad()
	{

	}

}