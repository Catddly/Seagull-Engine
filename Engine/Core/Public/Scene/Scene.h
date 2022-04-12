#pragma once

#include "Scene/Mesh/Mesh.h"
#include "Scene/Camera/ICamera.h"
#include "Scene/Light/PointLight.h"
#include "Scene/Light/DirectionalLight.h"

#include "TipECS/EntityManager.h"

#include "Stl/string.h"
#include "Stl/vector.h"
#include "Stl/unordered_map.h"
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
		Scene()
			:mpMainCamera(nullptr)
		{
		}

		//! This function can be dispatched to another thread. 
		SG_CORE_API void OnSceneLoad();
		SG_CORE_API void OnSceneUnLoad();

		SG_CORE_API void OnUpdate(float deltaTime);

		SG_CORE_API RefPtr<Mesh> GetMesh(const char* name);
		SG_CORE_API Size GetNumMesh() const { return mMeshes.size(); }
		SG_CORE_API const RefPtr<Mesh> GetSkybox() const { return mpSkyboxMesh; }

		DirectionalLight* GetDirectionalLight() { return mpDirectionalLight.get(); }
		ICamera* GetMainCamera() { return mpMainCamera.get(); }

		template <typename Func>
		SG_INLINE void TraverseMesh(Func&& func)
		{
			for (auto pNode : mMeshes)
				func(*pNode.second);
		}

		template <typename Func>
		SG_INLINE void TraverseEntity(Func&& func)
		{
			mEntityManager.TraverseEntity(eastl::forward<Func>(func));
		}

		template <typename Func>
		SG_INLINE void TraversePointLight(Func&& func)
		{
			for (auto& pointLight : mPointLights)
				func(pointLight);
		}
	private:
		void DefaultScene();
		void MaterialTestScene();
	private:
		TipECS::EntityManager<SGECSSetting> mEntityManager;

		// separate from scene mesh
		RefPtr<Mesh> mpSkyboxMesh;
		RefPtr<DirectionalLight> mpDirectionalLight;
		vector<PointLight>  mPointLights;
		// WHY it will fail to update mesh using TraverseMesh() when the type is unordered_map<string, Mesh>?
		unordered_map<string, RefPtr<Mesh>> mMeshes; // TODO: use tree structure to store the meshes (for now there are no many meshes in the scene, map is ok)

		RefPtr<ICamera>     mpMainCamera; // TODO: support multiply switchable camera
	};

}