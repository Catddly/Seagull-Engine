#pragma once

#include "TipECS/Config.h"
#include "TipECS/Registry.h"
#include "TipECS/Entity.h"
#include "Core/Private/TipECS/Components.h"
#include "TipECS/EntityHooker.h"

#if USE_STL
#	include <vector>
#else
#	include "eastl/vector.h"
#endif
#include <assert.h>

namespace TipECS
{

	template <typename TSetting>
	class EntityManager
	{
	private:
		using Setting = TSetting;
		using EntityHandle = Impl::EntityHandle<Setting>;
		using HandleData = Impl::HandleData;
		using EntityPrivateAccessor = Impl::EntityPrivateAccessor<Setting>;
		using SignatureBitSetsStorage = typename Setting::SignatureBitSetsStorage;
		using ComponentsStorage = typename ComponentsStorage<Setting>;
		using EntityHookerContainer = typename EntityHookerContainer<Setting>;
		using ThisType = EntityManager<Setting>;
	public:
		using Entity = Entity<Setting>;

		template <typename TSetting, typename TSignature>
		class SignatureIterator
		{
		private:
			using Setting = TSetting;
			using Signature = TSignature;
			using EntityManager = EntityManager<Setting>;
			using Entity = typename EntityManager::Entity;
			using ThisType = SignatureIterator<Setting, Signature>;
		public:
			SignatureIterator(EntityManager& manager)
				:mEntityManager(manager)
			{
				PreAdvance();
			}

			bool operator==(const SignatureIterator& rhs) const
			{
				return mCurrEntityID == rhs.mCurrEntityID;
			}

			bool operator!=(const SignatureIterator& rhs) const
			{
				return !operator==(rhs);
			}

			Entity operator*() const
			{
				Entity entity = {};
				mEntityManager.mEntityPrivateAccessor.GetHandleDataIndex(entity) = mEntityManager.GetEntityHandle(mCurrEntityID).handleDataIndex;
				mEntityManager.mEntityPrivateAccessor.GetCounterIndex(entity) = mEntityManager.GetHandleData(mCurrEntityID).counter;
				mEntityManager.mEntityPrivateAccessor.SetManagerPtr(entity, &mEntityManager);
				return entity;
			}

			ThisType& operator++()
			{
				Advance();
				return *this;
			}

			const ThisType begin() const
			{
				PreAdvance();
				return *this;
			}

			const ThisType end() const
			{
				mCurrEntityID = mEntityManager.mSizeNext;
				return *this;
			}
		private:
			void PreAdvance() const
			{
				while (!mEntityManager.IsAlive(mCurrEntityID) || !mEntityManager.MatchSignature<Signature>(mCurrEntityID))
				{
					++mCurrEntityID;
					if (mCurrEntityID == mEntityManager.mSizeNext)
						break;
				};
			}

			void Advance() const
			{
				do
				{
					++mCurrEntityID;
					if (mCurrEntityID == mEntityManager.mSizeNext)
						break;
				} while (!mEntityManager.IsAlive(mCurrEntityID) || !mEntityManager.MatchSignature<Signature>(mCurrEntityID));
			}
		private:
			EntityManager& mEntityManager;
			mutable EntityID mCurrEntityID{ 0 };
		};
	public:
		EntityManager()
			:mbCurrentFrameModified(true)
		{
			Reserve(100);
		}

		bool IsAlive(const Entity& entity) const noexcept
		{
			return IsAlive(GetEntityID(entity));
		}

		bool IsValid(const Entity& entity) const noexcept
		{
			// compare its counter with the handle data counter.
			return mEntityPrivateAccessor.GetHandleDataIndex(entity) != HandleDataIndex{ Impl::INVALID_INDEX } &&
				mEntityPrivateAccessor.GetCounterIndex(entity) == GetHandleData(entity).counter;
		}

		Entity CreateEntity() noexcept
		{
			mbCurrentFrameModified = true;
			return CreateEntityImpl();
		}

		void DestroyEntity(Entity& entity) noexcept
		{
			mbCurrentFrameModified = true;
			DestroyEntity(GetEntityID(entity));
			mEntityPrivateAccessor.SetManagerPtr(entity, nullptr);
		}

		template <typename TFunc>
		void TraverseEntity(TFunc&& func)
		{
			// don't traverse the newly created entity.
			for (EntityID i{ 0 }; i < mSizeNext; ++i)
			{
				if (IsAlive(i))
				{
					Entity entity = {};
					mEntityPrivateAccessor.GetHandleDataIndex(entity) = GetEntityHandle(i).handleDataIndex;
					mEntityPrivateAccessor.GetCounterIndex(entity) = GetHandleData(i).counter;
					mEntityPrivateAccessor.SetManagerPtr(entity, this);
					func(entity);
				}
			}
		}

		template <typename TSignature, typename TFunc>
		void TraverseEntityMatchSignature(TFunc&& func)
		{
			static_assert(Setting::template IsSignature<TSignature>(), "It is a not registered signature!");

			TraverseEntity([this, &func](const Entity& entity)
				{
					if (MatchSignature<TSignature>(GetEntityID(entity)))
						ExpandSignatureCall<TSignature>(entity, std::forward<TFunc>(func));
				});
		}

		template <typename... Ts>
		auto View() noexcept
		{
			using signature_t = typename TipECS::Signature<Ts...>;
			SignatureIterator<Setting, signature_t> iterator(*this);
			return iterator;
		}

		template <typename TComponent>
		bool HasComponent(const Entity& entity) const noexcept
		{
			return HasComponent<TComponent>(GetEntityID(entity));
		}

		template <typename TComponent>
		auto& AddComponent(const Entity& entity) noexcept
		{
			auto& comp = AddComponent<TComponent>(GetEntityID(entity));
			GetComponentHooker<TComponent>().OnComponentAdded(entity, comp);
			return comp;
		}

		template <typename TComponent, typename... Args>
		auto& AddComponent(const Entity& entity, Args&&... args) noexcept
		{
			auto& comp = AddComponent<TComponent>(GetEntityID(entity), FWD(args)...);
			GetComponentHooker<TComponent>().OnComponentAdded(entity, comp);
			return comp;
		}

		template <typename... Ts>
		decltype(auto) GetComponent(const Entity& entity) noexcept
		{
			static_assert(sizeof...(Ts) != 0, "GetComponent()'s template parameter can not be empty!");
			if constexpr (sizeof...(Ts) == 1)
			{
				return GetComponent<Ts...>(GetEntityID(entity));
			}
			else
			{
				// we can use structure binding here!
				auto entityID = GetEntityID(entity);
#ifdef USE_STL
				return std::tuple<decltype(GetComponent<Ts>(entityID))...>(GetComponent<Ts>(entityID)...);
#endif
			}
		}

		template <typename TComponent>
		void RemoveComponent(const Entity& entity) noexcept
		{
			auto& comp = GetComponent<TComponent>(entity);
			GetComponentHooker<TComponent>().OnComponentRemoved(entity, comp);
			RemoveComponent<TComponent>(GetEntityID(entity));
		}

		template <typename TComponent>
		auto& GetComponentHooker() noexcept
		{
			return mEntityHookerContainer.GetComponentHooker<TComponent>();
		}

		template <typename TTag>
		bool HasTag(const Entity& entity) const noexcept
		{
			return HasTag<TTag>(GetEntityID(entity));
		}

		template <typename TTag>
		void AddTag(const Entity& entity) noexcept
		{
			AddTag<TTag>(GetEntityID(entity));
			GetTagHooker<TTag>().OnTagAdded(entity);
		}

		template <typename TTag>
		void RemoveTag(const Entity& entity) noexcept
		{
			GetTagHooker<TTag>().OnTagRemoved(entity);
			RemoveTag<TTag>(GetEntityID(entity));
		}

		template <typename TComponent>
		auto& GetTagHooker() noexcept
		{
			return mEntityHookerContainer.GetTagHooker<TComponent>();
		}

		//! Clear all the entities and reset the status.
		void Clear() noexcept
		{
			for (size_t i = 0; i < mCapacity; ++i)
			{
				auto& entityHandle = mEntityHandles[i];
				entityHandle.bAlive = false;
				entityHandle.bitset.reset();
				entityHandle.dataIndex = i;
				entityHandle.handleDataIndex = i;

				auto& handleData = mHandleDatas[i];
				handleData.counter = 0;
				handleData.id = i;
			}
			mSize = mSizeNext = 0;
		}

		//! Reorder entities to make sure all the alive entities are at the front of the vector.
		//! Call it when you make sure user is not modifying the entities.
		void ReFresh() noexcept
		{
			if (mSizeNext == 0)
			{
				mSize = 0;
				return;
			}

			// if current frame had created or deleted entity, we have to do the refresh.
			// otherwise, save you some effort.
			if (mbCurrentFrameModified)
			{
				mSize = mSizeNext = ReFreshImpl();
				mbCurrentFrameModified = false;
			}
		}

		//! Get current capacity.
		size_t Capacity() const noexcept
		{
			return mCapacity;
		}

		//! Get current size.
		size_t Size() const noexcept
		{
			// here we return the mSize, so the newly created entities will not be able to know until user call the Refreseh().
			return mSize;
		}
	private:
		template <typename TFunc>
		void TraverseEntityID(TFunc&& func)
		{
			// don't traverse the newly created entity.
			for (EntityID i{ 0 }; i < mSize; ++i)
				func(i);
		}

		template <typename TSignature, typename TFunc>
		void TraverseEntityIDMatchSignature(TFunc&& func)
		{
			static_assert(Setting::template IsSignature<TSignature>(), "It is a not registered signature!");

			TraverseEntityID([this, &func](EntityID id)
				{
					if (MatchSignature<TSignature>(id))
						ExpandSignatureCall<TSignature>(id, std::forward<TFunc>(func));
				});
		}

		template <typename... Ts>
		struct UnpackedSignatureComponents;

		//! Expand the signature's parameters into the function's parameters.
		template <typename TSignature, typename TFunc>
		void ExpandSignatureCall(EntityID id, TFunc&& func)
		{
			static_assert(Setting::template IsSignature<TSignature>(), "It is a not registered signature!");
	
			using SignatureComponents = typename Setting::SignatureBitSets::template SignatureComponents<TSignature>;
			using UnpackedComponents = typename TMP::Unpack<UnpackedSignatureComponents, SignatureComponents>::type;

			UnpackedComponents::Call(id, *this, std::forward<TFunc>(func));
		};

		template <typename TSignature, typename TFunc>
		void ExpandSignatureCall(const Entity& entity, TFunc&& func)
		{
			static_assert(Setting::template IsSignature<TSignature>(), "It is a not registered signature!");

			using SignatureComponents = typename Setting::SignatureBitSets::template SignatureComponents<TSignature>;
			using UnpackedComponents = typename TMP::Unpack<UnpackedSignatureComponents, SignatureComponents>::type;

			UnpackedComponents::Call(entity, *this, std::forward<TFunc>(func));
		};

		template <typename... Ts>
		struct UnpackedSignatureComponents
		{
			template <typename TFunc>
			constexpr static void Call(EntityID id, ThisType& thisType, TFunc&& func)
			{
				auto dataIndex = thisType.GetEntityHandle(id).dataIndex;
				func(
					id,
					thisType.mComponentsStorage.GetComponent<Ts>(dataIndex)... // expand the components inside the function parameter list.
				);
			}

			template <typename TFunc>
			constexpr static void Call(const Entity& entity, ThisType& thisType, TFunc&& func)
			{
				auto dataIndex = thisType.GetEntityHandle(thisType.GetEntityID(entity)).dataIndex;
				func(
					thisType.mComponentsStorage.GetComponent<Ts>(dataIndex)... // expand the components inside the function parameter list.
				);
			}
		};

		//! Reorder algorithm implemented here.
		//! @return size of the current entities.
		EntityID ReFreshImpl() noexcept
		{
			EntityID indexAlive{ 0 }, indexDead{ mSizeNext - 1 };
			// two pointers swap algorithm (not stable)
			while (true)
			{
				// move the left pointer
				while (true)
				{
					if (indexAlive > indexDead)
						return indexAlive;
					if (!mEntityHandles[indexAlive].bAlive)
						break;
					++indexAlive;
				}
				// move the right pointer
				while (true)
				{
					if (mEntityHandles[indexDead].bAlive)
						break;
					// invalidate the dead entity handle data
					InvalidateEntity(indexDead);
					if (indexDead <= indexAlive)
						return indexAlive;
					--indexDead;
				}
				
				assert(!mEntityHandles[indexAlive].bAlive);
				assert(mEntityHandles[indexDead].bAlive);

				std::swap(mEntityHandles[indexAlive], mEntityHandles[indexDead]);
				RefreshEntity(indexAlive);
				InvalidateEntity(indexDead);
				RefreshEntity(indexDead);

				++indexAlive;
				--indexDead;
			}
			return indexAlive;
		}

		//! Free all the allocated memory.
		void FreeMemory() noexcept
		{
			mCapacity = mSize = mSizeNext = 0;
			mEntityHandles.resize(0);
			mEntityHandles.shrink_to_fit();
		}

		//! Preallocate the memory to expand the capacity to n.
		void Reserve(size_t n) noexcept
		{
			assert(n > mCapacity);

			mEntityHandles.resize(n); // resize to n, not reserve to n.
			mComponentsStorage.Reserve(n); // resize the storage of the components
			mHandleDatas.resize(n);
			
			for (size_t i = mCapacity; i < n; ++i) // init the newly created entity
			{
				EntityHandle& entityHandle = mEntityHandles[i];
				entityHandle.dataIndex = i;  // points to its column.
				entityHandle.bitset.reset(); // no components and tags
				entityHandle.bAlive = false; // dead
				entityHandle.handleDataIndex = i;

				HandleData& handleData = mHandleDatas[i];
				handleData.id = i;
				handleData.counter = 0;
			}

			mCapacity = n;
		}

		//! Grow to a new capacity if the size is not enough.
		void GrowIfNeeded() noexcept
		{
			if (mCapacity > mSizeNext)
				return;
			Reserve(mCapacity * 2);
		}
		
		EntityHandle& GetEntityHandle(EntityID id) noexcept
		{
			assert(mSizeNext > id); // user can immediately access the newly created entity.
			return mEntityHandles[id];
		}

		const EntityHandle& GetEntityHandle(EntityID id) const noexcept
		{
			assert(mSizeNext > id); // user can immediately access the newly created entity.
			return mEntityHandles[id];
		}
		//! Increase the counter so the handle data is invalid.
		void InvalidateEntity(EntityID id) noexcept
		{
			GetHandleData(id).counter++;
		}
		//! After the swap inside the RefreshImpl(), update handle data to its new entity.
		void RefreshEntity(EntityID id) noexcept
		{
			GetHandleData(id).id = id;
		}

		HandleData& GetHandleData(HandleDataIndex idx) noexcept
		{
			assert(idx < mHandleDatas.size());
			return mHandleDatas[idx];
		}

		const HandleData& GetHandleData(HandleDataIndex idx) const noexcept
		{
			assert(idx < mHandleDatas.size());
			return mHandleDatas[idx];
		}

		HandleData& GetHandleData(EntityID id) noexcept
		{
			return GetHandleData(GetEntityHandle(id).handleDataIndex);
		}

		const HandleData& GetHandleData(EntityID id) const noexcept
		{
			return GetHandleData(GetEntityHandle(id).handleDataIndex);
		}

		HandleData& GetHandleData(const Entity& entity) noexcept
		{
			return GetHandleData(mEntityPrivateAccessor.GetHandleDataIndex(entity));
		}

		const HandleData& GetHandleData(const Entity& entity) const noexcept
		{
			return GetHandleData(mEntityPrivateAccessor.GetHandleDataIndex(entity));
		}

		EntityID CreateEntityIndex() noexcept
		{
			GrowIfNeeded();

			size_t idx = mSizeNext++;
			EntityID availableIndex = EntityID(idx);

			assert(!IsAlive(availableIndex));

			auto& entity = GetEntityHandle(availableIndex);
			entity.bAlive = true;
			entity.bitset.reset();

			return availableIndex;
		}

		Entity CreateEntityImpl() noexcept
		{
			EntityID idx = CreateEntityIndex();
			assert(IsAlive(idx));

			EntityHandle& entityHandle = mEntityHandles[idx];
			HandleData&   handleData = mHandleDatas[entityHandle.handleDataIndex];

			handleData.id = idx; // update the entityHandle's handle data

			Entity entity = {};
			mEntityPrivateAccessor.GetHandleDataIndex(entity) = entityHandle.handleDataIndex;
			mEntityPrivateAccessor.GetCounterIndex(entity) = handleData.counter;
			mEntityPrivateAccessor.SetManagerPtr(entity, this);

			assert(IsValid(entity));
			return entity;
		}

		template <typename TSignature>
		bool MatchSignature(EntityID id) const noexcept
		{
			static_assert(Setting::template IsSignature<TSignature>(), "It is a not registered signature!");
			const auto& entityBitset = GetEntityHandle(id).bitset;
			const auto& signatureBitset = mSignatureBitSets.GetSignatureBitSet<TSignature>();

			return (entityBitset & signatureBitset) == signatureBitset;
		}
private:
		// EntityID relative functions
		EntityID GetEntityID(const Entity& entity) noexcept
		{
			assert(IsValid(entity));
			return GetHandleData(entity).id;
		}

		const EntityID GetEntityID(const Entity& entity) const noexcept
		{
			assert(IsValid(entity));
			return GetHandleData(entity).id;
		}

		bool IsAlive(EntityID id) const noexcept
		{
			return GetEntityHandle(id).bAlive;
		}

		void DestroyEntity(EntityID id) noexcept
		{
			// just mark it as dead, let the Refresh do its job.
			GetEntityHandle(id).bAlive = false;
		}

		template <typename TComponent>
		bool HasComponent(EntityID id) const noexcept
		{
			static_assert(Setting::template IsComponent<TComponent>(), "It is a not registered component!");
			return GetEntityHandle(id).bitset[Setting::template ComponentBitIndex<TComponent>()];
		}

		template <typename TComponent>
		auto& AddComponent(EntityID id) noexcept
		{
			static_assert(Setting::template IsComponent<TComponent>(), "It is a not registered component!");
			assert(!HasComponent<TComponent>(id));

			auto& entity = GetEntityHandle(id);
			entity.bitset[Setting::template ComponentBitIndex<TComponent>()] = true;

			return mComponentsStorage.AddComponent<TComponent>(entity.dataIndex);
		}

		template <typename TComponent, typename... Args>
		auto& AddComponent(EntityID id, Args&&... args) noexcept
		{
			static_assert(Setting::template IsComponent<TComponent>(), "It is a not registered component!");
			assert(!HasComponent<TComponent>(id));

			auto& entity = GetEntityHandle(id);
			entity.bitset[Setting::template ComponentBitIndex<TComponent>()] = true;

			auto& compData = mComponentsStorage.AddComponent<TComponent>(entity.dataIndex);
			// placement new to initialize the data
			new (&compData) TComponent(std::forward<Args>(args)...);
			return compData;
		}

		template <typename TComponent>
		auto& GetComponent(EntityID id) noexcept
		{
			static_assert(Setting::template IsComponent<TComponent>(), "It is a not registered component!");
			assert(HasComponent<TComponent>(id));

			return mComponentsStorage.GetComponent<TComponent>(GetEntityHandle(id).dataIndex);
		}

		template <typename TComponent>
		void RemoveComponent(EntityID id) noexcept
		{
			static_assert(Setting::template IsComponent<TComponent>(), "It is a not registered component!");
			GetEntityHandle(id).bitset[Setting::template ComponentBitIndex<TComponent>()] = false;
			mComponentsStorage.RemoveComponent<TComponent>(GetEntityHandle(id).dataIndex);
		}

		template <typename TTag>
		bool HasTag(EntityID id) const noexcept
		{
			static_assert(Setting::template IsTag<TTag>(), "It is a not registered tag!");
			return GetEntityHandle(id).bitset[Setting::template TagBitIndex<TTag>()];
		}

		template <typename TTag>
		void AddTag(EntityID id) noexcept
		{
			static_assert(Setting::template IsTag<TTag>(), "It is a not registered tag!");
			GetEntityHandle(id).bitset[Setting::template TagBitIndex<TTag>()] = true;
		}

		template <typename TTag>
		void RemoveTag(EntityID id) noexcept
		{
			static_assert(Setting::template IsTag<TTag>(), "It is a not registered tag!");
			GetEntityHandle(id).bitset[Setting::template TagBitIndex<TTag>()] = false;
		}
	private:
		//! Current capacity of the entity.
		size_t mCapacity = 0;
		//! Current size of the entity.
		size_t mSize = 0;
		//! Current new size of the entity, after the refresh is called, mSizeNext will equal to mSize.
		size_t mSizeNext = 0;
		//! Container of the entity handles.
#if USE_STL
		std::vector<EntityHandle> mEntityHandles;
#else
		eastl::vector<EntityHandle> mEntityHandles;
#endif
		//! Container of the entity.
#if USE_STL
		std::vector<HandleData> mHandleDatas;
#else
		eastl::vector<HandleData> mHandleDatas;
#endif
		//! Bitsets of the required signatures.
		SignatureBitSetsStorage mSignatureBitSets;
		//! Container of all the components data.
		ComponentsStorage mComponentsStorage;
		//! Used to access entity private member data.
		EntityPrivateAccessor mEntityPrivateAccessor;
		//! Used to hook some user custom callback events.
		EntityHookerContainer mEntityHookerContainer;
		//! Used to do some optimization.
		bool mbCurrentFrameModified;
	};

}