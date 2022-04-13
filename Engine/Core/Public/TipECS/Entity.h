#pragma once

#include "TipECS/Config.h"
#include "Core/Private/TipECS/TMPLib.h"

namespace TipECS
{

	// may be use strong typedef here to make sure size_t != DataIndex or EntityID.
	STRONG_TYPE_DEFS(size_t, DataIndex);
	STRONG_TYPE_DEFS(size_t, EntityID);
	// entity handle for the user
	STRONG_TYPE_DEFS(size_t, HandleDataIndex);
	STRONG_TYPE_DEFS(size_t, CounterIndex);

	namespace Impl
	{
		template <typename TSettings>
		struct EntityHandle
		{
			using Settings = TSettings;
			using BitSet = typename Settings::BitSet;
			//! Index to retrieve the actual component or tag data.
			DataIndex dataIndex;
			//! Handle data index to the handle array.
			HandleDataIndex handleDataIndex;
			//! Entity's own bitset to check if an entity has a component or tag
			BitSet bitset;
			//! To Check if this entity is alive(can be use by the user).
			bool bAlive;
		};

		struct HandleData
		{
			EntityID id;
			CounterIndex counter; //! Used to check if the handle is invalid as a version number.
		};

		// forward decoration
		template <typename TSetting>
		struct EntityPrivateAccessor;
	}

	template <typename TSetting>
	class EntityManager;

	template <typename TSetting>
	class Entity
	{
	private:
		using Setting = TSetting;
	public:
		template <typename TComponent>
		inline bool HasComponent() const noexcept
		{
			return pManager->HasComponent<TComponent>(*this);
		}

		template <typename TComponent>
		inline auto& AddComponent() noexcept
		{
			return pManager->AddComponent<TComponent>(*this);
		}

		template <typename TComponent, typename... Args>
		inline auto& AddComponent(Args&&... args) noexcept
		{
			return pManager->AddComponent<TComponent>(*this, FWD(args)...);
		}

		template <typename TComponent>
		inline void RemoveComponent() noexcept
		{
			pManager->RemoveComponent<TComponent>(*this);
		}

		template <typename... Ts>
		inline decltype(auto) GetComponent() noexcept
		{
			return pManager->GetComponent<Ts...>(*this);
		}

		template <typename TTag>
		bool HasTag() const noexcept
		{
			return pManager->HasTag<TTag>(*this);
		}

		template <typename TTag>
		void AddTag() noexcept
		{
			pManager->AddTag<TTag>(*this);
		}

		template <typename TTag>
		void RemoveTag() noexcept
		{
			pManager->RemoveTag<TTag>(*this);
		}
	private:
		friend struct TipECS::Impl::EntityPrivateAccessor<Setting>;
		EntityManager<Setting>* pManager = nullptr;
		HandleDataIndex handleDataIndex;
		CounterIndex counter; //! Used to check if the handle is invalid as a version number.
	};

	namespace Impl
	{
		template <typename TSetting>
		struct EntityPrivateAccessor
		{
			using Setting = TSetting;
			using Entity = Entity<Setting>;

			auto& GetHandleDataIndex(Entity& entity) { return entity.handleDataIndex; }
			auto& GetCounterIndex(Entity& entity) { return entity.counter; }

			const auto& GetHandleDataIndex(const Entity& entity) const { return entity.handleDataIndex; }
			const auto& GetCounterIndex(const Entity& entity) const { return entity.counter; }

			void              SetManagerPtr(Entity& entity, EntityManager<Setting>* pManager) { entity.pManager = pManager; }
			const auto* const GetManagerPtr(const Entity& entity) const { return entity.pManager; }
		};
	}

	// access entity data from the user should be like:
	// 1. Using the handleDataIndex stored in the Handle to get the HandleData.
	//
	// 2. If the counter doesn't match the counter of the HandleData, this handle is invalid.
	// 
	// 3. Otherwise, the data of the entity will be retrieve by the EntityID in the HandleData.

}