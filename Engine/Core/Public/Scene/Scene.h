#pragma once

#include "Archive/IDAllocator.h"
#include "Archive/ISerializable.h"
#include "Scene/Camera/ICamera.h"
#include "Scene/Components.h"

#include "TipECS/EntityManager.h"

#include "Stl/string.h"
#include "Stl/vector.h"
#include "Stl/unordered_map.h"
#include "eastl/array.h"
#include "eastl/list.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	// not support deferred or forward+ render pipeline for now
	// so, 10 point light is enough.
#define SG_MAX_NUM_POINT_LIGHT 10

	class Scene final : public ISerializable
	{
	public:
		using EntityManager = typename TipECS::EntityManager<SGECSSetting>;
		using Entity = typename TipECS::Entity<SGECSSetting>;

		struct SceneTreeNode
		{
			Entity* pEntity = nullptr;
			SceneTreeNode* pParent = nullptr;
			eastl::vector<SceneTreeNode*> pChilds;

			SceneTreeNode() = default;
			SceneTreeNode(Entity* ptr)
				:pEntity(ptr), pParent(nullptr)
			{}
			~SceneTreeNode() = default;
		};

		using TreeNode = SceneTreeNode;

		struct EntityContext
		{
			Entity    entity = {};
			TreeNode* pTreeNode = nullptr;
		};

		using EntityContext = EntityContext;
	public:
		Scene();
		~Scene();

		//! This function can be dispatched to another thread. 
		SG_CORE_API void OnSceneLoad();
		SG_CORE_API void OnSceneUnLoad();

		SG_CORE_API void OnUpdate(float deltaTime);

		SG_CORE_API Entity* CreateEntity(const string& name);
		SG_CORE_API Entity* CreateEntity(const string& name, const Vector3f& pos, const Vector3f& scale, const Vector3f& rot);

		SG_CORE_API Entity* CreateEntityWithMesh(const string& name, const string& filename, EMeshType type, ELoadMeshFlag flag = ELoadMeshFlag(0));

		SG_CORE_API void    DestroyEntity(Entity& entity);
		SG_CORE_API void    DestroyEntityByName(const string& name);

		SG_CORE_API EntityContext* GetEntityContextByName(const string& name);
		SG_CORE_API Entity* GetEntityByName(const string& name);

		SG_CORE_API Entity  GetSkyboxEntity() { return mSkyboxEntity; }

		Entity& GetMainCamera() { return *mpCameraEntity; }

		SG_CORE_API Size GetMeshEntityCount() const { return mMeshEntityCount; }
		SG_CORE_API Size GetEntityCount()     const { return mEntityContexts.size(); }

		SG_CORE_API TreeNode* GetTreeRepresentation() const { return mpRootNode; };

		template <typename... Ts>
		SG_INLINE auto View()
		{
			return mEntityManager.View<Ts...>();
		}

		template <typename Func>
		SG_INLINE void TraverseEntity(Func&& func)
		{
			for (auto node : mEntityContexts)
				func(node.second.entity);
		}

		template <typename Func>
		SG_INLINE void TraverseEntityContext(Func&& func)
		{
			for (auto node : mEntityContexts)
				func(node.second);
		}
	private:
		// entity creation and destruction
		EntityContext* CreateEntityContextWithoutTreeNode(const string& name);

		EntityContext* CreateEntityContext(const string& name);
		EntityContext* CreateEntityContext(const string& name, const Vector3f& pos, const Vector3f& scale, const Vector3f& rot);

		EntityContext* CreateEntityContextWithMesh(const string& name, const string& filename, EMeshType type, ELoadMeshFlag flag = ELoadMeshFlag(0));
	private:
		void DefaultScene();
		void MaterialScene();
		void MaterialTexturedScene();

		void UpdateMeshAABB();

		virtual void Serialize(json& node) override;
		virtual void Deserialize(json& node) override;
		void SerializeEntity(Scene::TreeNode* pTreeNode, json& node);
		void DeserializeEntity(Scene::TreeNode* pTreeNode, json& entity);

		void Refresh();

		void ClearTreeNodes();
		void FreeTreeNode(TreeNode* pTreeNode);
	private:
		Entity  mSkyboxEntity;
		Entity* mpCameraEntity;  // TODO: support multiply switchable camera

		Size mMeshEntityCount = 0;
		unordered_map<string, EntityContext> mEntityContexts; //! Contain all the entities' contexts in the scene. name -> EntityContext

		//! Tree representation of the scene.
		TreeNode* mpRootNode;

		EntityManager mEntityManager;
	};

	// helper functions

	static Matrix4f GetTransform(Scene::TreeNode* pTreeNode)
	{
		if (!pTreeNode->pParent->pEntity)
			return GetTransform(pTreeNode->pEntity->GetComponent<TransformComponent>());
		
		if (pTreeNode->pParent)
			return GetTransform(pTreeNode->pParent) * GetTransform(pTreeNode->pEntity->GetComponent<TransformComponent>());
		return Matrix4f(1.0f);
	}

}