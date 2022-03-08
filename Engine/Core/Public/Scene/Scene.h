#pragma once

#include "Scene/Mesh/Mesh.h"
#include "Scene/Camera/ICamera.h"
#include "Scene/Light/PointLight.h"
#include "Scene/Light/DirectionalLight.h"

#include "Stl/string.h"
#include "Stl/vector.h"
#include "eastl/array.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	// not support deferred or forward+ render pipeline for now
	// so, 10 point light is enough.
#define SG_MAX_NUM_POINT_LIGHT 10

	class Scene
	{
	public:
		Scene(const char* name)
			:mName(name), mpMainCamera(nullptr), mDirectionalLight({ 4.0f, 7.0f, 2.0f }, { -2.0f, -5.0f, -1.0f }, { 1.0f, 1.0f, 1.0f })
		{}

		//! This function can be dispatched to another thread. 
		SG_CORE_API void OnSceneLoad();
		SG_CORE_API void OnSceneUnLoad();

		DirectionalLight* GetDirectionalLight() { return &mDirectionalLight; }
		ICamera* GetMainCamera() { return mpMainCamera.get(); }

		template <typename Func>
		void TraverseMesh(Func&& func)
		{
			for (auto& mesh : mMeshes)
				func(mesh);
		}

		template <typename Func>
		void TraversePointLight(Func&& func)
		{
			for (auto& pointLight : mPointLights)
				func(pointLight);
		}
	private:
		string mName;

		DirectionalLight   mDirectionalLight;
		vector<PointLight> mPointLights;
		vector<Mesh>       mMeshes;      // TODO: use tree structure to store the meshes

		RefPtr<ICamera>    mpMainCamera; // TODO: support multiply switchable camera
	};

}