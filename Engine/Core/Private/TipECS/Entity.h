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
		struct EntityPrivateAccessor;
	}

	class Entity
	{
	private:
		friend struct TipECS::Impl::EntityPrivateAccessor;
		HandleDataIndex handleDataIndex;
		CounterIndex counter; //! Used to check if the handle is invalid as a version number.
	};

	namespace Impl
	{
		struct EntityPrivateAccessor
		{
			auto& GetHandleDataIndex(Entity& entity) { return entity.handleDataIndex; }
			auto& GetCounterIndex(Entity& entity) { return entity.counter; }

			const auto& GetHandleDataIndex(const Entity& entity) const { return entity.handleDataIndex; }
			const auto& GetCounterIndex(const Entity& entity) const { return entity.counter; }
		};
	}

	// access entity data from the user should be like:
	// 1. Using the handleDataIndex stored in the Handle to get the HandleData.
	//
	// 2. If the counter doesn't match the counter of the HandleData, this handle is invalid.
	// 
	// 3. Otherwise, the data of the entity will be retrieve by the EntityID in the HandleData.

}