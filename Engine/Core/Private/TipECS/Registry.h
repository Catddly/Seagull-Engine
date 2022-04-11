#pragma once

#include "TipECS/Config.h"
#include "Core/Private/TipECS/TMPLib.h"

#ifdef USE_STL
#	include <tuple>
#endif
#include <bitset>
#include <iostream>

namespace TipECS
{

	// Collection of components
	template <typename... TComponents>
#ifdef USE_STL
	using CompoentList = TMP::TypeList<TComponents...>;
#endif

	// Collection of tags
	template <typename... TTags>
#ifdef USE_STL
	using TagList = TMP::TypeList<TTags...>;
#endif

	// Collection of signatures
	template <typename... Ts>
#ifdef USE_STL
	using Signature = TMP::TypeList<Ts...>;
#endif

	// Collection of signatures
	template <typename... TSignatures>
#ifdef USE_STL
	using SignatureList = TMP::TypeList<TSignatures...>;
#endif

	// forward decoration
	namespace Impl
	{
		template <typename TSetting>
		struct SignatureBitSets;

		template <typename TSetting>
		class SignatureBitSetsStorage;
	}

	template <typename TComponentList, typename TTagList, typename TSignatureList>
	struct Setting
	{
		// force the type of the tag must be 0 size.
		template <typename TTag>
		using ZeroSizeTagFilter = std::integral_constant<bool, std::is_empty_v<TTag>>;
		
		using ComponentList = TComponentList;
		using TagList       = TTagList;
		using SignatureList = TSignatureList;

		using FilteredTags = typename TMP::Filter<ZeroSizeTagFilter, TagList>::type;
		static_assert(FilteredTags::Size() == TagList::Size(), "Non-zero size tag found! Tag must be empty struct!");

		using ThisType = Setting<ComponentList, TagList, SignatureList>;

		using SignatureBitSets = Impl::SignatureBitSets<ThisType>;
		using SignatureBitSetsStorage = Impl::SignatureBitSetsStorage<ThisType>;

		template <typename T>
		static constexpr size_t ComponentID() noexcept
		{
			return ComponentList::template At<T>();
		}

		template <typename T>
		static constexpr size_t TagID() noexcept
		{
			return TagList::template At<T>();
		}

		template <typename T>
		static constexpr size_t SignatureID() noexcept
		{
			return SignatureList::template At<T>();
		}

		template <typename T>
		static constexpr bool IsComponent() noexcept
		{
			return ComponentList::template Contain<T>();
		}

		template <typename T>
		static constexpr bool IsTag() noexcept
		{
			return TagList::template Contain<T>();
		}

		template <typename T>
		static constexpr bool IsSignature() noexcept
		{
			return SignatureList::template Contain<T>();
		}

		static constexpr size_t ComponentCount() noexcept
		{
			return ComponentList::Size();
		}

		static constexpr size_t TagCount() noexcept
		{
			return TagList::Size();
		}

		static constexpr size_t SignatureCount() noexcept
		{
			return SignatureList::Size();
		}

		// construct bitset
		// the front bits of the bitset is component and the back bits of the bitset is tags
		using BitSet = std::bitset<ComponentCount() + TagCount()>;

		//! Return the index of the bit in the bitset.
		template <typename T>
		static constexpr size_t ComponentBitIndex() noexcept
		{
			return ComponentID<T>();
		}

		//! Return the index of the bit in the bitset.
		template <typename T>
		static constexpr size_t TagBitIndex() noexcept
		{
			return ComponentCount() + TagID<T>();
		}
	};

	namespace Impl
	{

		//! Bitsets for each signature in the signature list.
		template <typename TSetting>
		struct SignatureBitSets
		{
			using Setting = TSetting;
			using SignatureList = typename Setting::SignatureList;
			using BitSet = typename Setting::BitSet;
			using ThisType = SignatureBitSets<Setting>;

			// n copy of BitSet in a tuple
			using RepeatBitSet = decltype(TMP::MakeTupleN<Setting::SignatureCount(), BitSet>());

			// filter the components of signatures in compile time
			template <typename TComponent>
			using ComponentFilter = std::integral_constant<bool, Setting::template IsComponent<TComponent>()>;

			// filter the tags of signatures in compile time
			template <typename TTag>
			using TagFilter = std::integral_constant<bool, Setting::template IsTag<TTag>()>;

			// separate components from signature 
			template <typename TSignature>
			using SignatureComponents = typename TMP::Filter<ComponentFilter, TSignature>::type;

			// separate tags from signature 
			template <typename TSignature>
			using SignatureTags = typename TMP::Filter<TagFilter, TSignature>::type;
		};

		template <typename TSetting>
		class SignatureBitSetsStorage
		{
		private:
			using Setting = TSetting;
			using SignatureBitSets = typename Setting::SignatureBitSets;
			using SignatureList = typename Setting::SignatureList;
			using RepeatBitSet = typename SignatureBitSets::RepeatBitSet;
		public:
			SignatureBitSetsStorage() noexcept
			{
				// init all the bitsets of signatures
				TMP::Apply<SignatureList>([this](auto... t)
					{
						// dirty way to expand the template parameters inside a function.
						using _dummy = int[];
						(void)_dummy { 0, (this->InitSignaturesBitSets<decltype(t)>(), 0)... };
					});
			}

			//! Return the bitset of a given signature
			template <typename TSignature>
			auto& GetSignatureBitSet() noexcept
			{
				static_assert(Setting::template IsSignature<TSignature>(), "TSignature is not a signature!");
#ifdef USE_STL
				return std::get<Setting::template SignatureID<TSignature>()>(mSignatureBitSets);
#endif
			}

			//! Return the bitset of a given signature
			template <typename TSignature>
			const auto& GetSignatureBitSet() const noexcept
			{
				static_assert(Setting::template IsSignature<TSignature>(), "TSignature is not a signature!");
#ifdef USE_STL
				return std::get<Setting::template SignatureID<TSignature>()>(mSignatureBitSets);
#endif
			}
		private:
			//! Initialize the bitset for each signature at run-time.
			// TODO: may be initialize this at compile-time in the future?
			template <typename TSignature>
			void InitSignaturesBitSets() noexcept
			{
				auto& bitset = GetSignatureBitSet<TSignature>();
				
				using SignatureComponents = typename SignatureBitSets::template SignatureComponents<TSignature>;
				using SignatureTags = typename SignatureBitSets::template SignatureTags<TSignature>;

				// init bits of the components
				TMP::Apply<SignatureComponents>([this, &bitset](auto... t)
					{
						// dirty way to expand the template parameters inside a function.
						using _dummy = int[];
						(void)_dummy { 0, (bitset[Setting::template ComponentBitIndex<decltype(t)>()] = true, 0)... };
					});
				// init bits of the tags
				TMP::Apply<SignatureTags>([this, &bitset](auto... t)
					{
						// dirty way to expand the template parameters inside a function.
						using _dummy = int[];
						(void)_dummy { 0, (bitset[Setting::template TagBitIndex<decltype(t)>()] = true, 0)... };
					});
			}
		private:
			RepeatBitSet mSignatureBitSets;
		};

	}

}