#pragma once

#include "TipECS/Config.h"
#include "TipECS/Entity.h"
#include "TipECS/Registry.h"

#include <tuple>
#if USE_STL
#	include <vector>
#else
#	include "eastl/vector.h"
#endif

namespace TipECS
{

	template <typename TSettings>
	class ComponentsStorage
	{
	private:
		using Setting = TSettings;
		using ComponentList = typename Setting::ComponentList;

		// we use sparse set to store the actual data index to prevent the memory waste scenario.
		template <typename TComponent>
		struct ComponentData
		{
#if USE_STL
			std::vector<size_t>     dataIndices;    //! Array contain the index of the compData array.
			std::vector<TComponent> compData;       //! Array which stores all the component data.
			std::vector<size_t>     validCompIndex; //! Stack which contain the free index of the compData array.
#else
			eastl::vector<size_t>     dataIndices;    //! Array contain the index of the compData array.
			eastl::vector<TComponent> compData;       //! Array which stores all the component data.
			eastl::vector<size_t>     validCompIndex; //! Stack which contain the free index of the compData array.
#endif
		};

		template <typename... Ts>
		using TupleOfVectors = std::tuple<ComponentData<Ts>...>;

		// this is a std::tuple<ComponentData<Ts>...>, each component data refers to a component type.
		// it is used for indexing the data to the packed data array.
		using ComponentsType = typename TMP::Unpack<TupleOfVectors, ComponentList>::type;
	public:
		ComponentsStorage() noexcept
		{
			// initialized size of compData is 2.
			// here we make sure at the start of the ECS, validCompIndex is not empty.
			TMP::ForTuple(mComponentsData, [](auto& component)
				{
					component.compData.resize(2);
					component.validCompIndex.push_back(1);
					component.validCompIndex.push_back(0);
				});
		}

		template <typename TComponent>
		auto& AddComponent(DataIndex idx) noexcept
		{
			static_assert(Setting::template IsComponent<TComponent>(), "TComponent is not registered!");
			auto& component = std::get<ComponentData<TComponent>>(mComponentsData);

			// check to see if the compData can store more data
			if (component.validCompIndex.empty()) // do resize
			{
				auto prevSize = component.compData.size();
				component.compData.resize(prevSize * 2); // double the size
				for (size_t i = component.compData.size() - 1; i >= prevSize; --i) // push the new index into the validCompIndex 
					component.validCompIndex.push_back(i);
			}

			auto validIndex = component.validCompIndex.back();
			component.validCompIndex.pop_back();
			// store the valid index in the dataIndices array, so next time user can get it.
			component.dataIndices[idx] = validIndex;
			return component.compData[validIndex];
		}

		template <typename TComponent>
		void RemoveComponent(DataIndex idx)
		{
			static_assert(Setting::template IsComponent<TComponent>(), "TComponent is not registered!");
			auto& component = std::get<ComponentData<TComponent>>(mComponentsData);

			component.validCompIndex.push_back(component.dataIndices[idx]);
		}

		template <typename TComponent>
		auto& GetComponent(DataIndex idx) noexcept
		{
			static_assert(Setting::template IsComponent<TComponent>(), "TComponent is not registered!");

			auto& component = std::get<ComponentData<TComponent>>(mComponentsData);
			return component.compData[component.dataIndices[idx]];
		}

		void Reserve(size_t n)
		{
			std::apply([this, n](auto&... v)
				{
					using _dummy = int[];
					(void)_dummy {
						0, (v.dataIndices.resize(n), 0)...
					};
				}, mComponentsData);
		}
	private:
		ComponentsType mComponentsData;
	};

}