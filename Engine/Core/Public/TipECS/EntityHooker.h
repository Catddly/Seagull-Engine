#pragma once

#include "TipECS/Config.h"
#include "TipECS/Registry.h"

#include "Core/Private/TipECS/TMPLib.h"

#include <tuple>

namespace TipECS
{

	// forward decoration
	template <typename TSetting>
	class EntityHookerContainer;

	template <typename TSetting>
	class EntityManager;

	template <typename TSetting, typename TComponent>
	class EntityComponentHooker
	{
	private:
		using Setting = TSetting;
		static_assert(Setting::template IsComponent<TComponent>(), "TComponent is not a registered component!");
		using TComp = TComponent;
	public:
		using Entity = Entity<Setting>;
		typedef void(*ComponentHookFunc)(const Entity& entity, TComp& comp);

		inline void HookOnAdded(ComponentHookFunc func) noexcept { if (func) mOnAddedFunc = func; }
		inline void HookOnRemoved(ComponentHookFunc func) noexcept { if (func) mOnRemovedFunc = func; }
	private:
		friend class EntityHookerContainer<Setting>;
		friend class EntityManager<Setting>;
		inline void OnComponentAdded(const Entity& entity, TComp& comp) noexcept
		{
			if (mOnAddedFunc)
				mOnAddedFunc(entity, comp);
		}

		inline void OnComponentRemoved(const Entity& entity, TComp& comp) noexcept
		{
			if (mOnRemovedFunc)
				mOnRemovedFunc(entity, comp);
		}
	private:
		ComponentHookFunc mOnAddedFunc = nullptr;
		ComponentHookFunc mOnRemovedFunc = nullptr;
	};

	template <typename TSetting, typename TTag>
	class EntityTagHooker
	{
	private:
		using Setting = TSetting;
		static_assert(Setting::template IsTag<TTag>(), "TTag is not a registered tag!");
		using Tag = TTag;
	public:
		using Entity = Entity<Setting>;
		typedef void(*TagHookFunc)(const Entity& entity);

		inline void HookOnAdded(TagHookFunc func) noexcept { if (func) mOnAddedFunc = func; }
		inline void HookOnRemoved(TagHookFunc func) noexcept { if (func) mOnRemovedFunc = func; }
	private:
		friend class EntityHookerContainer<Setting>;
		friend class EntityManager<Setting>;
		inline void OnTagAdded(const Entity& entity) noexcept
		{
			if (mOnAddedFunc)
				mOnAddedFunc(entity);
		}

		inline void OnTagRemoved(const Entity& entity) noexcept
		{
			if (mOnRemovedFunc)
				mOnRemovedFunc(entity);
		}
	private:
		TagHookFunc mOnAddedFunc = nullptr;
		TagHookFunc mOnRemovedFunc = nullptr;
	};

	template <typename TSetting>
	class EntityHookerContainer
	{
	private:
		using Setting = TSetting;
		using ComponentList = typename Setting::ComponentList;
		using TagList = typename Setting::TagList;

		template <typename TComponent>
		using ComponentHookerType = EntityComponentHooker<Setting, TComponent>;
		template <typename TTag>
		using TagHookerType = EntityTagHooker<Setting, TTag>;

		template <typename... Ts>
		using ComponentsTuple = std::tuple<ComponentHookerType<Ts>...>;
		template <typename... Ts>
		using TagsTuple = std::tuple<TagHookerType<Ts>...>;

		using ComponentHookers = typename TMP::Unpack<ComponentsTuple, ComponentList>::type;
		using TagHookers = typename TMP::Unpack<TagsTuple, TagList>::type;
	public:
		template <typename TComponent>
		constexpr ComponentHookerType<TComponent>& GetComponentHooker() noexcept
		{
			static_assert(Setting::template IsComponent<TComponent>());
			return std::get<ComponentHookerType<TComponent>>(mComponentHookers);
		}

		template <typename TTag>
		constexpr TagHookerType<TTag>& GetTagHooker() noexcept
		{
			static_assert(Setting::template IsTag<TTag>());
			return std::get<TagHookerType<TTag>>(mTagHookers);
		}
	private:
		ComponentHookers mComponentHookers;
		TagHookers mTagHookers;
	};

}