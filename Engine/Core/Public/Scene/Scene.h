#pragma once

#include "Scene/Camera/ICamera.h"
#include "Scene/Components.h"

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
		using EntityManager = typename TipECS::EntityManager<SGECSSetting>;
		using Entity = typename TipECS::Entity<SGECSSetting>;
	public:
		Scene()
			:mpMainCamera(nullptr)
		{
		}

		//! This function can be dispatched to another thread. 
		SG_CORE_API void OnSceneLoad();
		SG_CORE_API void OnSceneUnLoad();

		SG_CORE_API void OnUpdate(float deltaTime);

		SG_CORE_API Entity* CreateEntity(const string& name);
		SG_CORE_API Entity* CreateEntity(const string& name, const Vector3f& pos, const Vector3f& scale, const Vector3f& rot);

		SG_CORE_API void    DestroyEntity(Entity& entity);
		SG_CORE_API void    DestroyEntityByName(const string& name);

		SG_CORE_API Entity* GetEntityByName(const string& name);

		SG_CORE_API Entity  GetSkyboxEntity() { return mSkyboxEntity; }

		ICamera* GetMainCamera() { return mpMainCamera.get(); }

		SG_CORE_API Size GetMeshEntityCount() const { return mMeshEntityCount; }
		SG_CORE_API Size GetEntityCount()     const { return mEntities.size(); }

		template <typename... Ts>
		SG_INLINE auto View()
		{
			return mEntityManager.View<Ts...>();
		}

		template <typename Func>
		SG_INLINE void TraverseEntity(Func&& func)
		{
			for (auto node : mEntities)
				func(node.second);
		}
	private:
		static UInt32 NewObjectID() noexcept
		{
			return gCurrObjectID++;
		}

		void DefaultScene();
		void MaterialScene();
		void MaterialTexturedScene();
	private:
		Entity mSkyboxEntity;
		RefPtr<ICamera> mpMainCamera; // TODO: support multiply switchable camera

		// TODO: use tree structure to store the meshes (for now there are no many meshes in the scene, map is ok)
		Size mMeshEntityCount = 0;
		unordered_map<string, Entity> mEntities;

		EntityManager mEntityManager;
		static UInt32 gCurrObjectID;
	};

}